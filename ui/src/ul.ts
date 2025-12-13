class Ultralight {
  private static _instance: Ultralight | null = null;
  private _isAvailable = false;

  private constructor() {}

  static get instance(): Ultralight {
    if (!this._instance) {
      this._instance = new Ultralight();
    }
    
    return this._instance;
  }

  get isAvailable(): boolean {
    return this._isAvailable;
  }

  getFbxFileFormatVersion(): string {
    if (!this.isAvailable) {
      throw new Error("Ultralight is not available.");
    }
    return (window as any).__ul_getFbxFileFormatVersion();
  }
}

const ultralight = Ultralight.instance;

export default ultralight;