import { FBXNode } from "./models/fbxnode";

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

  getFBXNode(id: number): FBXNode {
    if (!this.isAvailable) {
      throw new Error("Ultralight is not available.");
    }
    return (window as any).__ul_getFBXNode(id);
  }

  getFBXNodeChildren(id: number): number[] {
    if (!this.isAvailable) {
      throw new Error("Ultralight is not available.");
    }
    return (window as any).__ul_getFBXNodeChildren(id);
  }

  selectFbxFile(): boolean {
    if (!this.isAvailable) {
      throw new Error("Ultralight is not available.");
    }
    return (window as any).__ul_SelectFbxFile();
  }

  closeWindow(): void {
    if (!this.isAvailable) {
      throw new Error("Ultralight is not available.");
    }
    (window as any).__ul_CloseWindow();
  }
  
  openAboutDialog(): void {
    if (!this.isAvailable) {
      throw new Error("Ultralight is not available.");
    }
    (window as any).__ul_OpenAboutDialog();
  }

  getAppVersion(): string {
    if (!this.isAvailable) {
      throw new Error("Ultralight is not available.");
    }
    return (window as any).__ul_getAppVersion();
  }
}

const ultralight = Ultralight.instance;

export default ultralight;
