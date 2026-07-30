const DB_NAME = 'YJStudyDB';
const DB_VERSION = 1;

class DataStore {
  constructor() {
    this.db = null;
    this.initPromise = this.initDB();
  }

  initDB() {
    return new Promise((resolve, reject) => {
      const req = indexedDB.open(DB_NAME, DB_VERSION);
      req.onerror = () => reject(req.error);
      req.onsuccess = () => { this.db = req.result; resolve(); };
      req.onupgradeneeded = (e) => {
        const db = e.target.result;
        if (!db.objectStoreNames.contains('chapters')) {
          db.createObjectStore('chapters', { keyPath: 'id' });
        }
        if (!db.objectStoreNames.contains('questions')) {
          const qs = db.createObjectStore('questions', { keyPath: 'id' });
          qs.createIndex('chapter', 'chapter', { unique: false });
          qs.createIndex('section', 'section', { unique: false });
          qs.createIndex('isWrong', 'isWrong', { unique: false });
          qs.createIndex('attempts', 'attempts', { unique: false });
        }
        if (!db.objectStoreNames.contains('records')) {
          db.createObjectStore('records', { keyPath: 'id', autoIncrement: true });
        }
        if (!db.objectStoreNames.contains('settings')) {
          db.createObjectStore('settings', { keyPath: 'key' });
        }
      };
    });
  }

  async ensureReady() {
    if (!this.db) await this.initPromise;
  }

  async get(store, key) {
    await this.ensureReady();
    return new Promise((resolve, reject) => {
      const tx = this.db.transaction(store, 'readonly');
      const req = tx.objectStore(store).get(key);
      req.onsuccess = () => resolve(req.result);
      req.onerror = () => reject(req.error);
    });
  }

  async getAll(store) {
    await this.ensureReady();
    return new Promise((resolve, reject) => {
      const tx = this.db.transaction(store, 'readonly');
      const req = tx.objectStore(store).getAll();
      req.onsuccess = () => resolve(req.result);
      req.onerror = () => reject(req.error);
    });
  }

  async put(store, data) {
    await this.ensureReady();
    return new Promise((resolve, reject) => {
      const tx = this.db.transaction(store, 'readwrite');
      const req = tx.objectStore(store).put(data);
      req.onsuccess = () => resolve(req.result);
      req.onerror = () => reject(req.error);
    });
  }

  async delete(store, key) {
    await this.ensureReady();
    return new Promise((resolve, reject) => {
      const tx = this.db.transaction(store, 'readwrite');
      const req = tx.objectStore(store).delete(key);
      req.onsuccess = () => resolve();
      req.onerror = () => reject(req.error);
    });
  }

  async query(store, indexName, value) {
    await this.ensureReady();
    return new Promise((resolve, reject) => {
      const tx = this.db.transaction(store, 'readonly');
      const idx = tx.objectStore(store).index(indexName);
      const req = idx.getAll(value);
      req.onsuccess = () => resolve(req.result);
      req.onerror = () => reject(req.error);
    });
  }

  // Chapters
  async getChapters() {
    const all = await this.getAll('chapters');
    return all.sort((a, b) => (a.order || 0) - (b.order || 0));
  }

  async saveChapter(chapter) {
    return this.put('chapters', chapter);
  }

  async deleteChapter(id) {
    return this.delete('chapters', id);
  }

  // Questions
  async getQuestions() {
    return this.getAll('questions');
  }

  async getQuestionsByChapter(chapter) {
    return this.query('questions', 'chapter', chapter);
  }

  async getWrongQuestions() {
    return this.query('questions', 'isWrong', true);
  }

  async saveQuestion(q) {
    return this.put('questions', q);
  }

  async saveQuestions(questions) {
    for (const q of questions) await this.saveQuestion(q);
  }

  // Records
  async getRecords() {
    const all = await this.getAll('records');
    return all.sort((a, b) => (b.date || 0) - (a.date || 0));
  }

  async addRecord(record) {
    record.date = record.date || Date.now();
    return this.put('records', record);
  }

  // Settings
  async getSetting(key, defaultValue) {
    const s = await this.get('settings', key);
    return s ? s.value : defaultValue;
  }

  async setSetting(key, value) {
    return this.put('settings', { key, value });
  }
}

const store = new DataStore();
