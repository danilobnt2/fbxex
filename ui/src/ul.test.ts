import { describe, it, expect, vi, beforeEach, afterEach } from "vitest";

const BRIDGE_ERROR = "Ultralight is not available.";

const loadUltralight = async () => {
  vi.resetModules();
  return (await import("./ul")).default as any;
};

describe("ultralight bridge", () => {
  let ul: any;

  beforeEach(async () => {
    ul = await loadUltralight();
  });

  afterEach(() => {
    const win = window as any;
    delete win.__ul_getFbxFileFormatVersion;
    delete win.__ul_getFBXNode;
    delete win.__ul_getFBXNodeChildren;
    delete win.__ul_SelectFbxFile;
    delete win.__ul_CloseWindow;
    delete win.__ul_OpenAboutDialog;
    delete win.__ul_getAppVersion;
  });

  it("returns the same singleton instance across imports", async () => {
    const secondImport = (await import("./ul")).default;

    expect(secondImport).toBe(ul);
    expect(ul.isAvailable).toBe(false);
  });

  it("rejects bridge calls when Ultralight is unavailable", () => {
    const expectUnavailable = (fn: () => unknown) => expect(fn).toThrow(BRIDGE_ERROR);

    expectUnavailable(() => ul.getFbxFileFormatVersion());
    expectUnavailable(() => ul.getFBXNode(1));
    expectUnavailable(() => ul.getFBXNodeChildren(2));
    expectUnavailable(() => ul.selectFbxFile());
    expectUnavailable(() => ul.closeWindow());
    expectUnavailable(() => ul.openAboutDialog());
    expectUnavailable(() => ul.getAppVersion());
  });

  it("delegates to Ultralight window bridge when available", () => {
    const win = window as any;
    win.__ul_getFbxFileFormatVersion = vi.fn(() => "7.4.0");
    win.__ul_getFBXNode = vi.fn((id: number) => ({ id, props: { name: `Node ${id}` } }));
    win.__ul_getFBXNodeChildren = vi.fn((id: number) => [id + 1, id + 2]);
    win.__ul_SelectFbxFile = vi.fn(() => true);
    win.__ul_CloseWindow = vi.fn();
    win.__ul_OpenAboutDialog = vi.fn();
    win.__ul_getAppVersion = vi.fn(() => "1.2.3");

    (ul as any)._isAvailable = true;

    expect(ul.isAvailable).toBe(true);
    expect(ul.getFbxFileFormatVersion()).toBe("7.4.0");
    expect(win.__ul_getFbxFileFormatVersion).toHaveBeenCalledTimes(1);

    expect(ul.getFBXNode(10)).toEqual({ id: 10, props: { name: "Node 10" } });
    expect(win.__ul_getFBXNode).toHaveBeenCalledWith(10);

    expect(ul.getFBXNodeChildren(5)).toEqual([6, 7]);
    expect(win.__ul_getFBXNodeChildren).toHaveBeenCalledWith(5);

    expect(ul.selectFbxFile()).toBe(true);
    expect(win.__ul_SelectFbxFile).toHaveBeenCalledTimes(1);

    ul.closeWindow();
    expect(win.__ul_CloseWindow).toHaveBeenCalledTimes(1);

    ul.openAboutDialog();
    expect(win.__ul_OpenAboutDialog).toHaveBeenCalledTimes(1);

    expect(ul.getAppVersion()).toBe("1.2.3");
    expect(win.__ul_getAppVersion).toHaveBeenCalledTimes(1);
  });
});
