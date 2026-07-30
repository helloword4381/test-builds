class QuizEngine {
  constructor(dataStore) {
    this.store = dataStore;
  }

  async generateDailyQuiz(chapterId, sectionId, mode = 'review') {
    const allQuestions = await this.store.getQuestions();
    const today = this._dateKey(Date.now());
    const yesterday = this._dateKey(Date.now() - 86400000);

    if (mode === 'practice' && sectionId) {
      // 随堂课练: 当前小节10道题
      const sectionQs = allQuestions.filter(q => q.section === sectionId);
      return this._shuffle(sectionQs).slice(0, 10);
    }

    if (mode === 'review') {
      // 第二天复习: 50道题
      const result = [];
      const usedIds = new Set();

      // 1. 10道预习题 - 下一节内容
      const nextSectionQs = await this._getNextSectionQuestions(allQuestions, chapterId, sectionId);
      const previewQs = this._shuffle(nextSectionQs).slice(0, 10);
      previewQs.forEach(q => { q.quizTag = '预习题'; usedIds.add(q.id); result.push(q); });

      // 2. 10道昨天刚做过的题
      const yesterdayQs = allQuestions.filter(q => q.lastAttemptDate && this._dateKey(q.lastAttemptDate) === yesterday);
      const recentQs = this._shuffle(yesterdayQs).slice(0, 10);
      recentQs.forEach(q => { if (!usedIds.has(q.id)) { q.quizTag = '昨日回顾'; usedIds.add(q.id); result.push(q); } });

      // 3. 10道错题
      const wrongQs = allQuestions.filter(q => q.isWrong);
      const wrongSelected = this._shuffle(wrongQs).slice(0, 10);
      wrongSelected.forEach(q => { if (!usedIds.has(q.id)) { q.quizTag = '错题重做'; usedIds.add(q.id); result.push(q); } });

      // 4. 20道做题次数最少的题
      const remaining = allQuestions.filter(q => !usedIds.has(q.id));
      const leastAttempted = remaining.sort((a, b) => (a.attempts || 0) - (b.attempts || 0)).slice(0, 20);
      leastAttempted.forEach(q => { q.quizTag = '薄弱练习'; result.push(q); });

      return result.slice(0, 50);
    }

    return [];
  }

  async _getNextSectionQuestions(allQuestions, chapterId, currentSectionId) {
    const chapters = await this.store.getChapters();
    const chapter = chapters.find(c => c.id === chapterId);
    if (!chapter || !chapter.sections) return [];
    const idx = chapter.sections.findIndex(s => s.id === currentSectionId);
    const nextSection = chapter.sections[idx + 1];
    if (!nextSection) return [];
    return allQuestions.filter(q => q.section === nextSection.id);
  }

  async submitAnswer(question, selectedOptions, timeSpent = 0) {
    const isCorrect = this._checkAnswer(question, selectedOptions);
    question.attempts = (question.attempts || 0) + 1;
    question.lastAttemptDate = Date.now();
    if (isCorrect) {
      question.correctAttempts = (question.correctAttempts || 0) + 1;
      question.isWrong = false;
    } else {
      question.isWrong = true;
    }
    await this.store.saveQuestion(question);
    await this.store.addRecord({
      type: 'quiz',
      questionId: question.id,
      correct: isCorrect,
      selected: selectedOptions,
      timeSpent,
      date: Date.now()
    });
    return { correct: isCorrect, answer: question.answer };
  }

  _checkAnswer(question, selected) {
    if (!Array.isArray(selected)) selected = [selected];
    const correct = Array.isArray(question.answer) ? question.answer : [question.answer];
    if (selected.length !== correct.length) return false;
    return selected.every(s => correct.includes(s)) && correct.every(c => selected.includes(c));
  }

  async getStats() {
    const questions = await this.store.getQuestions();
    const total = questions.length;
    const wrong = questions.filter(q => q.isWrong).length;
    const attempted = questions.filter(q => q.attempts > 0).length;
    const todayKey = this._dateKey(Date.now());
    const todayCount = questions.filter(q => q.lastAttemptDate && this._dateKey(q.lastAttemptDate) === todayKey).length;
    return { total, wrong, attempted, today: todayCount };
  }

  async getWrongBook(filters = {}) {
    let questions = await this.store.getWrongQuestions();
    if (filters.chapter) questions = questions.filter(q => q.chapter === filters.chapter);
    return questions;
  }

  _shuffle(arr) {
    const a = [...arr];
    for (let i = a.length - 1; i > 0; i--) {
      const j = Math.floor(Math.random() * (i + 1));
      [a[i], a[j]] = [a[j], a[i]];
    }
    return a;
  }

  _dateKey(ts) {
    const d = new Date(ts);
    return `${d.getFullYear()}-${String(d.getMonth()+1).padStart(2,'0')}-${String(d.getDate()).padStart(2,'0')}`;
  }
}

const quizEngine = new QuizEngine(store);
