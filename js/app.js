const App = {
  currentPage: 'course',
  currentChapter: null,
  currentSection: null,
  quizQuestions: [],
  quizIndex: 0,
  quizAnswers: {},
  studySeconds: 0,
  restSeconds: 600,
  restInterval: null,
  studyInterval: null,
  speakText: '',

  init() {
    this.bindNav();
    this.bindGlobalEvents();
    this.startStudyTimer();
    this.loadTheme();
    this.navigate('course');
  },

  bindNav() {
    document.querySelectorAll('.nav-item').forEach(item => {
      item.addEventListener('click', (e) => {
        e.preventDefault();
        const page = item.dataset.page;
        this.navigate(page);
      });
    });
  },

  bindGlobalEvents() {
    document.getElementById('btn-theme').addEventListener('click', () => this.toggleTheme());
    document.getElementById('btn-speak').addEventListener('click', () => this.speak());
    document.querySelector('.modal-close').addEventListener('click', () => this.closeModal());
    document.getElementById('modal-overlay').addEventListener('click', (e) => {
      if (e.target.id === 'modal-overlay') this.closeModal();
    });
  },

  navigate(page) {
    this.currentPage = page;
    document.querySelectorAll('.nav-item').forEach(i => i.classList.toggle('active', i.dataset.page === page));
    document.getElementById('page-title').textContent = {
      course: '课程学习', quiz: '每日一练', plan: '学习计划',
      wrong: '错题本', import: '资料导入', settings: '设置'
    }[page];
    const content = document.getElementById('content');
    content.innerHTML = '';
    switch(page) {
      case 'course': this.renderCourse(); break;
      case 'quiz': this.renderQuizSetup(); break;
      case 'plan': this.renderPlan(); break;
      case 'wrong': this.renderWrongBook(); break;
      case 'import': this.renderImport(); break;
      case 'settings': this.renderSettings(); break;
    }
  },

  // ==================== Course ====================
  async renderCourse() {
    const content = document.getElementById('content');
    const chapters = await store.getChapters();
    if (!chapters.length) {
      content.innerHTML = `<div class="card text-center"><p>暂无课程数据，请先导入资料</p><button class="btn btn-primary mt-4" onclick="App.navigate('import')">去导入</button></div>`;
      return;
    }
    content.innerHTML = `<div class="course-grid"><div class="card"><div class="card-title">课程目录</div><div class="chapter-list" id="chapter-list"></div></div><div id="course-main"></div></div>`;
    this.renderChapterList(chapters);
    if (chapters[0]?.sections?.length) {
      this.loadSection(chapters[0].id, chapters[0].sections[0].id);
    }
  },

  renderChapterList(chapters) {
    const list = document.getElementById('chapter-list');
    list.innerHTML = chapters.map(ch => `
      <div class="chapter-item">
        <div class="chapter-header" onclick="App.toggleChapter('${ch.id}')">
          <span>${ch.title}</span>
          <span>${ch.sections?.length || 0}节</span>
        </div>
        <div class="section-list hidden" id="sections-${ch.id}">
          ${(ch.sections || []).map(s => `
            <div class="section-item ${this.currentSection === s.id ? 'active' : ''}" onclick="App.loadSection('${ch.id}', '${s.id}')">
              <span>${s.title}</span>
              <span class="section-status ${s.completed ? 'done' : ''}">${s.completed ? '已完成' : '未学'}</span>
            </div>
          `).join('')}
        </div>
      </div>
    `).join('');
  },

  toggleChapter(chId) {
    const el = document.getElementById(`sections-${chId}`);
    el.classList.toggle('hidden');
  },

  async loadSection(chId, secId) {
    this.currentChapter = chId;
    this.currentSection = secId;
    const chapters = await store.getChapters();
    const chapter = chapters.find(c => c.id === chId);
    const section = chapter?.sections?.find(s => s.id === secId);
    if (!section) return;

    document.querySelectorAll('.section-item').forEach(el => el.classList.remove('active'));
    event?.target?.closest('.section-item')?.classList.add('active');

    const main = document.getElementById('course-main');
    main.innerHTML = `
      <div class="card">
        <div class="flex gap-2 mb-4">
          <button class="btn btn-primary btn-sm" onclick="App.playVideo('${section.video || ''}')">播放视频</button>
          <button class="btn btn-secondary btn-sm" onclick="App.showNotes('${section.id}')">查看讲义</button>
          <button class="btn btn-success btn-sm" onclick="App.startPractice('${chId}', '${secId}')">随堂课练 (10题)</button>
          <button class="btn btn-warning btn-sm" onclick="App.startRest()">休息10分钟</button>
          <button class="btn btn-secondary btn-sm" onclick="App.markCompleted('${chId}', '${secId}')">标记完成</button>
        </div>
        <div id="video-player" class="video-area hidden"></div>
        <div id="notes-area" class="pdf-area hidden">${section.notes || '暂无讲义内容，请导入讲义资料'}</div>
      </div>
      <div class="card">
        <div class="card-title">本节要点</div>
        <div id="key-points">${section.keyPoints || '暂无要点，请导入讲义资料'}</div>
      </div>
    `;
    this.speakText = section.keyPoints || '';
  },

  playVideo(url) {
    const player = document.getElementById('video-player');
    if (!url) { alert('本节暂无视频，请导入课程资料'); return; }
    player.innerHTML = `<video controls autoplay src="${url}"></video>`;
    player.classList.remove('hidden');
  },

  showNotes(secId) {
    const area = document.getElementById('notes-area');
    area.classList.toggle('hidden');
  },

  async startPractice(chId, secId) {
    this.quizQuestions = await quizEngine.generateDailyQuiz(chId, secId, 'practice');
    if (!this.quizQuestions.length) { alert('本节暂无题目，请先导入题库'); return; }
    this.quizIndex = 0;
    this.quizAnswers = {};
    this.renderQuizSession('随堂课练 - 10道题');
  },

  async markCompleted(chId, secId) {
    const chapters = await store.getChapters();
    const ch = chapters.find(c => c.id === chId);
    const sec = ch?.sections?.find(s => s.id === secId);
    if (sec) { sec.completed = true; await store.saveChapter(ch); this.renderCourse(); }
  },

  // ==================== Quiz ====================
  async renderQuizSetup() {
    const content = document.getElementById('content');
    const stats = await quizEngine.getStats();
    content.innerHTML = `
      <div class="quiz-setup">
        <div class="quiz-stats">
          <div class="stat-card card"><div class="stat-value">${stats.total}</div><div class="stat-label">总题数</div></div>
          <div class="stat-card card"><div class="stat-value">${stats.attempted}</div><div class="stat-label">已做题</div></div>
          <div class="stat-card card"><div class="stat-value">${stats.wrong}</div><div class="stat-label">错题数</div></div>
          <div class="stat-card card"><div class="stat-value">${stats.today}</div><div class="stat-label">今日做题</div></div>
        </div>
        <div class="card">
          <div class="card-title">选择练习模式</div>
          <div class="flex gap-4 mt-4">
            <button class="btn btn-primary" onclick="App.startDailyReview()">每日复习 (50题)</button>
            <button class="btn btn-secondary" onclick="App.startWrongPractice()">错题练习</button>
            <button class="btn btn-secondary" onclick="App.startRandomPractice()">随机练习</button>
          </div>
        </div>
      </div>
    `;
  },

  async startDailyReview() {
    if (!this.currentChapter || !this.currentSection) {
      alert('请先选择课程章节，系统将根据当前进度生成复习题');
      this.navigate('course');
      return;
    }
    this.quizQuestions = await quizEngine.generateDailyQuiz(this.currentChapter, this.currentSection, 'review');
    if (!this.quizQuestions.length) { alert('题库为空，请先导入题目'); return; }
    this.quizIndex = 0;
    this.quizAnswers = {};
    this.renderQuizSession('每日复习 - 50道题');
  },

  async startWrongPractice() {
    const wrong = await quizEngine.getWrongBook();
    if (!wrong.length) { alert('暂无错题，太棒了！'); return; }
    this.quizQuestions = wrong;
    this.quizIndex = 0;
    this.quizAnswers = {};
    this.renderQuizSession('错题练习');
  },

  async startRandomPractice() {
    const all = await store.getQuestions();
    if (!all.length) { alert('题库为空'); return; }
    this.quizQuestions = all.sort(() => Math.random() - 0.5).slice(0, 20);
    this.quizIndex = 0;
    this.quizAnswers = {};
    this.renderQuizSession('随机练习 - 20题');
  },

  renderQuizSession(title) {
    const content = document.getElementById('content');
    const q = this.quizQuestions[this.quizIndex];
    const typeLabel = { single: '单选题', multiple: '多选题', truefalse: '判断题' }[q.type] || '选择题';
    const optionsHtml = (q.options || []).map((opt, i) => `
      <div class="option ${this.quizAnswers[q.id]?.includes(opt[0]) ? 'selected' : ''}" data-opt="${opt[0]}" onclick="App.selectOption('${q.id}', '${opt[0]}')">
        <span class="option-text">${opt}</span>
      </div>
    `).join('');

    content.innerHTML = `
      <div class="quiz-container">
        <div class="quiz-progress">
          <span>${title}</span>
          <span>第 ${this.quizIndex + 1} / ${this.quizQuestions.length} 题</span>
        </div>
        ${q.quizTag ? `<span class="question-type">${q.quizTag}</span>` : ''}
        <div class="question-card card">
          <div class="question-content">${q.content}</div>
          <div class="options">${optionsHtml}</div>
        </div>
        <div class="quiz-actions">
          <button class="btn btn-secondary" ${this.quizIndex === 0 ? 'disabled' : ''} onclick="App.prevQuestion()">上一题</button>
          <button class="btn btn-primary" onclick="App.nextQuestion()">${this.quizIndex === this.quizQuestions.length - 1 ? '提交' : '下一题'}</button>
        </div>
      </div>
    `;
    this.speakText = q.content;
  },

  selectOption(qid, opt) {
    const q = this.quizQuestions.find(x => x.id === qid);
    if (!this.quizAnswers[qid]) this.quizAnswers[qid] = [];
    if (q.type === 'single' || q.type === 'truefalse') {
      this.quizAnswers[qid] = [opt];
    } else {
      const idx = this.quizAnswers[qid].indexOf(opt);
      if (idx > -1) this.quizAnswers[qid].splice(idx, 1);
      else this.quizAnswers[qid].push(opt);
    }
    this.renderQuizSession(document.querySelector('.quiz-progress span')?.textContent || '练习');
  },

  prevQuestion() {
    if (this.quizIndex > 0) { this.quizIndex--; this.renderQuizSession(document.querySelector('.quiz-progress span')?.textContent || '练习'); }
  },

  async nextQuestion() {
    if (this.quizIndex < this.quizQuestions.length - 1) {
      this.quizIndex++;
      this.renderQuizSession(document.querySelector('.quiz-progress span')?.textContent || '练习');
    } else {
      await this.submitQuiz();
    }
  },

  async submitQuiz() {
    let correct = 0;
    for (const q of this.quizQuestions) {
      const ans = this.quizAnswers[q.id] || [];
      const res = await quizEngine.submitAnswer(q, ans);
      if (res.correct) correct++;
    }
    const score = Math.round((correct / this.quizQuestions.length) * 100);
    const content = document.getElementById('content');
    content.innerHTML = `
      <div class="quiz-result card">
        <div class="result-score">${score}分</div>
        <div class="result-detail">答对 ${correct} / ${this.quizQuestions.length} 题</div>
        <div class="flex gap-4 mt-4" style="justify-content:center">
          <button class="btn btn-primary" onclick="App.renderQuizSetup()">返回题库</button>
          <button class="btn btn-secondary" onclick="App.startRest()">休息10分钟</button>
        </div>
      </div>
    `;
  },

  // ==================== Plan ====================
  async renderPlan() {
    const content = document.getElementById('content');
    const today = new Date();
    const daysInMonth = new Date(today.getFullYear(), today.getMonth() + 1, 0).getDate();
    const firstDay = new Date(today.getFullYear(), today.getMonth(), 1).getDay();
    let calendarHtml = '';
    for (let i = 0; i < firstDay; i++) calendarHtml += `<div></div>`;
    for (let d = 1; d <= daysInMonth; d++) {
      const isToday = d === today.getDate();
      calendarHtml += `<div class="plan-day ${isToday ? 'today' : ''}"><span class="plan-day-number">${d}</span></div>`;
    }

    const chapters = await store.getChapters();
    const totalSections = chapters.reduce((sum, c) => sum + (c.sections?.length || 0), 0);
    const completedSections = chapters.reduce((sum, c) => sum + (c.sections?.filter(s => s.completed)?.length || 0), 0);
    const progress = totalSections ? Math.round((completedSections / totalSections) * 100) : 0;

    content.innerHTML = `
      <div class="card">
        <div class="card-title">本月学习日历</div>
        <div class="plan-calendar">${calendarHtml}</div>
      </div>
      <div class="card">
        <div class="card-title">学习进度</div>
        <div class="mb-4">总体进度: ${completedSections}/${totalSections} 节 (${progress}%)</div>
        <div id="plan-tasks"></div>
      </div>
    `;
    this.renderPlanTasks(chapters);
  },

  renderPlanTasks(chapters) {
    const container = document.getElementById('plan-tasks');
    const nextUncompleted = chapters.flatMap(c => (c.sections || []).map(s => ({...s, chapterTitle: c.title}))).find(s => !s.completed);
    const tasks = [];
    if (nextUncompleted) {
      tasks.push({ tag: '新课', content: `学习: ${nextUncompleted.chapterTitle} - ${nextUncompleted.title}` });
      tasks.push({ tag: '讲义', content: `阅读对应讲义并标记重点` });
      tasks.push({ tag: '练习', content: `完成随堂课练 10 道题` });
    }
    tasks.push({ tag: '复习', content: `每日复习 50 道题（预习+回顾+错题+薄弱）` });

    container.innerHTML = `<div class="task-list">${tasks.map((t, i) => `
      <div class="task-item">
        <div class="task-checkbox" onclick="this.classList.toggle('checked')">${''}</div>
        <div class="task-content">${t.content}</div>
        <span class="task-tag">${t.tag}</span>
      </div>
    `).join('')}</div>`;
  },

  // ==================== Wrong Book ====================
  async renderWrongBook() {
    const content = document.getElementById('content');
    const wrong = await quizEngine.getWrongBook();
    if (!wrong.length) {
      content.innerHTML = `<div class="card text-center"><p>暂无错题，继续保持！</p></div>`;
      return;
    }
    content.innerHTML = `
      <div class="card">
        <div class="card-title">错题本 (${wrong.length}题)</div>
        <div id="wrong-list"></div>
      </div>
    `;
    this.renderWrongList(wrong);
  },

  renderWrongList(questions) {
    const list = document.getElementById('wrong-list');
    list.innerHTML = questions.slice(0, 50).map((q, i) => `
      <div class="task-item" style="margin-bottom:8px;cursor:pointer" onclick="App.showWrongDetail('${q.id}')">
        <div class="task-content">${i+1}. ${q.content.substring(0, 60)}${q.content.length>60?'...':''}</div>
        <span class="task-tag">${q.chapter || '未分类'}</span>
        <button class="btn btn-sm btn-success" onclick="event.stopPropagation();App.removeWrong('${q.id}')">移出</button>
      </div>
    `).join('');
  },

  async showWrongDetail(qid) {
    const q = (await store.getQuestions()).find(x => x.id === qid);
    if (!q) return;
    this.showModal('错题详情', `
      <div class="question-content">${q.content}</div>
      <div class="options" style="margin-bottom:16px">${(q.options||[]).map(o => `<div class="option ${q.answer.includes(o[0])?'correct':''}">${o}</div>`).join('')}</div>
      <div><strong>解析:</strong> ${q.explanation || '暂无解析'}</div>
    `);
  },

  async removeWrong(qid) {
    const q = (await store.getQuestions()).find(x => x.id === qid);
    if (q) { q.isWrong = false; await store.saveQuestion(q); this.renderWrongBook(); }
  },

  // ==================== Import ====================
  renderImport() {
    const content = document.getElementById('content');
    content.innerHTML = `
      <div class="card">
        <div class="card-title">导入课程章节</div>
        <textarea id="import-chapters" rows="6" placeholder='粘贴 JSON 格式的章节数据'></textarea>
        <button class="btn btn-primary mt-4" onclick="App.importChapters()">导入章节</button>
      </div>
      <div class="card">
        <div class="card-title">导入题库</div>
        <textarea id="import-questions" rows="6" placeholder='粘贴 JSON 格式的题目数据'></textarea>
        <button class="btn btn-primary mt-4" onclick="App.importQuestions()">导入题目</button>
      </div>
      <div class="card">
        <div class="card-title">导入讲义内容</div>
        <div class="form-group">
          <label class="form-label">选择章节</label>
          <select id="notes-chapter"><option>请先导入章节</option></select>
        </div>
        <div class="form-group">
          <label class="form-label">选择小节</label>
          <select id="notes-section"><option>请先选择章节</option></select>
        </div>
        <textarea id="import-notes" rows="6" placeholder='粘贴讲义文本内容'></textarea>
        <button class="btn btn-primary mt-4" onclick="App.importNotes()">导入讲义</button>
      </div>
      <div class="card import-format">
        <div class="card-title">资料投喂格式说明</div>
        <p>点击下方按钮查看完整的资料整理规范，按规范准备好数据后粘贴到上方对应区域即可导入。</p>
        <button class="btn btn-secondary" onclick="App.showFormatGuide()">查看格式规范</button>
      </div>
    `;
    this.loadChapterSelect();
  },

  async loadChapterSelect() {
    const chapters = await store.getChapters();
    const chSel = document.getElementById('notes-chapter');
    if (!chSel) return;
    chSel.innerHTML = chapters.map(c => `<option value="${c.id}">${c.title}</option>`).join('');
    chSel.onchange = () => {
      const ch = chapters.find(c => c.id === chSel.value);
      const secSel = document.getElementById('notes-section');
      secSel.innerHTML = (ch?.sections || []).map(s => `<option value="${s.id}">${s.title}</option>`).join('');
    };
    if (chapters[0]) chSel.onchange();
  },

  async importChapters() {
    try {
      const data = JSON.parse(document.getElementById('import-chapters').value);
      const chapters = Array.isArray(data) ? data : [data];
      for (const ch of chapters) await store.saveChapter(ch);
      alert(`成功导入 ${chapters.length} 个章节`);
      this.loadChapterSelect();
    } catch(e) { alert('格式错误: ' + e.message); }
  },

  async importQuestions() {
    try {
      const data = JSON.parse(document.getElementById('import-questions').value);
      const questions = Array.isArray(data) ? data : [data];
      await store.saveQuestions(questions);
      alert(`成功导入 ${questions.length} 道题目`);
    } catch(e) { alert('格式错误: ' + e.message); }
  },

  async importNotes() {
    const chId = document.getElementById('notes-chapter').value;
    const secId = document.getElementById('notes-section').value;
    const notes = document.getElementById('import-notes').value;
    const chapters = await store.getChapters();
    const ch = chapters.find(c => c.id === chId);
    const sec = ch?.sections?.find(s => s.id === secId);
    if (sec) { sec.notes = notes; await store.saveChapter(ch); alert('讲义导入成功'); }
  },

  showFormatGuide() {
    this.showModal('资料投喂格式规范', `
      <div style="max-height:400px;overflow-y:auto">
        <h4>1. 章节数据格式 (JSON)</h4>
        <pre>[\n  {\n    "id": "ch1",\n    "title": "第1章 建筑工程技术",\n    "order": 1,\n    "sections": [\n      {\n        "id": "s1-1",\n        "title": "1.1 建筑构造要求",\n        "video": "video/1-1.mp4",\n        "keyPoints": "要点文本...",\n        "completed": false\n      }\n    ]\n  }\n]</pre>
        <h4>2. 题目数据格式 (JSON)</h4>
        <pre>[\n  {\n    "id": "q001",\n    "type": "single",\n    "chapter": "第1章",\n    "section": "s1-1",\n    "content": "题目内容",\n    "options": ["A. 选项1", "B. 选项2", "C. 选项3", "D. 选项4"],\n    "answer": ["A"],\n    "explanation": "解析内容"\n  }\n]</pre>
        <h4>3. 支持题型</h4>
        <ul>\n          <li>single - 单选题</li>\n          <li>multiple - 多选题</li>\n          <li>truefalse - 判断题</li>\n        </ul>
        <h4>4. 说明</h4>
        <p>视频路径可以是相对路径（本地文件）或完整URL。导入后数据保存在浏览器本地，不会上传。</p>
      </div>
    `);
  },

  // ==================== Settings ====================
  renderSettings() {
    const content = document.getElementById('content');
    content.innerHTML = `
      <div class="card">
        <div class="card-title">学习设置</div>
        <div class="form-group">
          <label class="form-label">朗读语速 (0.5-2)</label>
          <input type="number" id="speak-rate" min="0.5" max="2" step="0.1" value="1">
        </div>
        <div class="form-group">
          <label class="form-label">每日目标题数</label>
          <input type="number" id="daily-goal" min="10" max="200" value="50">
        </div>
        <button class="btn btn-primary" onclick="App.saveSettings()">保存设置</button>
        <button class="btn btn-secondary" onclick="App.exportData()">导出所有数据</button>
        <button class="btn btn-danger" onclick="App.clearAllData()">清空所有数据</button>
      </div>
    `;
  },

  async saveSettings() {
    await store.setSetting('speakRate', parseFloat(document.getElementById('speak-rate').value));
    await store.setSetting('dailyGoal', parseInt(document.getElementById('daily-goal').value));
    alert('设置已保存');
  },

  async exportData() {
    const data = {
      chapters: await store.getChapters(),
      questions: await store.getQuestions(),
      records: await store.getRecords(),
      exportDate: new Date().toISOString()
    };
    const blob = new Blob([JSON.stringify(data, null, 2)], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url; a.download = `yj-study-backup-${Date.now()}.json`; a.click();
    URL.revokeObjectURL(url);
  },

  async clearAllData() {
    if (!confirm('确定要清空所有学习数据吗？此操作不可恢复！')) return;
    indexedDB.deleteDatabase(DB_NAME);
    location.reload();
  },

  // ==================== Timer & Rest ====================
  startStudyTimer() {
    this.studyInterval = setInterval(() => {
      this.studySeconds++;
      const min = Math.floor(this.studySeconds / 60);
      document.getElementById('study-today').textContent = `今日学习: ${min}分钟`;
    }, 60000);
  },

  startRest() {
    if (this.restInterval) return;
    this.restSeconds = 600;
    const timerEl = document.getElementById('rest-timer');
    timerEl.classList.remove('hidden');
    this.restInterval = setInterval(() => {
      this.restSeconds--;
      const m = Math.floor(this.restSeconds / 60);
      const s = this.restSeconds % 60;
      document.querySelector('.rest-time').textContent = `${m}:${String(s).padStart(2,'0')}`;
      if (this.restSeconds <= 0) {
        clearInterval(this.restInterval);
        this.restInterval = null;
        timerEl.classList.add('hidden');
        alert('休息结束，继续加油！');
      }
    }, 1000);
  },

  // ==================== Speak ====================
  async speak() {
    const text = this.speakText || document.getElementById('key-points')?.textContent || '';
    if (!text) return;
    const rate = await store.getSetting('speakRate', 1);
    const utter = new SpeechSynthesisUtterance(text);
    utter.lang = 'zh-CN';
    utter.rate = rate;
    speechSynthesis.cancel();
    speechSynthesis.speak(utter);
  },

  // ==================== Theme ====================
  loadTheme() {
    const theme = localStorage.getItem('theme') || 'light';
    document.documentElement.setAttribute('data-theme', theme);
  },

  toggleTheme() {
    const current = document.documentElement.getAttribute('data-theme') || 'light';
    const next = current === 'light' ? 'dark' : 'light';
    document.documentElement.setAttribute('data-theme', next);
    localStorage.setItem('theme', next);
  },

  // ==================== Modal ====================
  showModal(title, html) {
    document.getElementById('modal-title').textContent = title;
    document.getElementById('modal-body').innerHTML = html;
    document.getElementById('modal-overlay').classList.remove('hidden');
  },

  closeModal() {
    document.getElementById('modal-overlay').classList.add('hidden');
  }
};

document.addEventListener('DOMContentLoaded', () => App.init());
