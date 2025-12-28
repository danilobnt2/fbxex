import React from "react";
import { Provider } from "react-redux";
import { describe, it, beforeEach, afterEach, expect, vi } from "vitest";
import { render, screen, waitFor } from "@testing-library/react";

import { createAppStore } from "./store";
import { ITreePlatform } from "./viewmodels/treeview";
import { FBXNodeProps } from "./models/fbxnode";

const mockUl = {
  isAvailable: false,
  selectFbxFile: vi.fn(),
  closeWindow: vi.fn(),
  openAboutDialog: vi.fn(),
  getAppVersion: vi.fn(() => "mock-version"),
  getFBXNode: vi.fn((id: number) => ({ props: { name: `Node ${id}`, properties: [], attributes: [] } })),
  getFBXNodeChildren: vi.fn(() => [] as number[]),
};

vi.mock("./ul", () => ({ default: mockUl }));

vi.mock("@heroui/system", () => ({
  HeroUIProvider: ({ children }: any) => <div data-testid="heroui-provider">{children}</div>,
}));

vi.mock("@heroui/input", () => ({
  Textarea: ({ value, ...props }: any) => <textarea data-testid="properties-textarea" readOnly value={value} {...props} />,
}));

vi.mock("./containers", () => ({
  MenuBar: (props: any) => <div data-testid="menu-bar">menu {Object.keys(props).join(",")}</div>,
  HierarchyTreeView: (props: any) => <div data-testid="hierarchy-tree">tree {Object.keys(props).join(",")}</div>,
}));

class MockTreePlatform implements ITreePlatform {
  isPlatformAvailable = () => true;
  getFBXNodeChildren = () => [];
  getFBXNodeProperties = () => ({ name: "Mock node", properties: [], attributes: [] });
  getFBXPreviewProperties = () => ({ name: "Mock node" });
}

const createStore = () => createAppStore(new MockTreePlatform());

const loadRoot = async () => (await import("./main")).Root;

describe("main entry", () => {
  beforeEach(() => {
    vi.clearAllMocks();
    vi.resetModules();
    document.body.innerHTML = "";
    delete (window as any).__remountApp;
  });

  afterEach(() => {
    delete (window as any).__remountApp;
  });

  it("renders with fallback app version when Ultralight is unavailable", async () => {
    mockUl.isAvailable = false;
    const Root = await loadRoot();
    const store = createStore();

    render(
      <Provider store={store}>
        <Root />
      </Provider>
    );

    expect(screen.getByText("fbxex dev")).toBeInTheDocument();
    expect(screen.getByText("Ready")).toBeInTheDocument();
    expect(screen.getByTestId("properties-textarea")).toHaveValue("{}");
  });

  it("uses the Ultralight app version and cleans up remount handler", async () => {
    mockUl.isAvailable = true;
    mockUl.getAppVersion.mockReturnValueOnce("2.4.6");
    const Root = await loadRoot();
    const store = createStore();

    const { unmount } = render(
      <Provider store={store}>
        <Root />
      </Provider>
    );

    expect(screen.getByText("fbxex 2.4.6")).toBeInTheDocument();
    expect(typeof window.__remountApp).toBe("function");

    window.__remountApp?.();
    await waitFor(() => {
      expect(screen.getByText("fbxex 2.4.6")).toBeInTheDocument();
    });

    unmount();
    await waitFor(() => {
      expect(window.__remountApp).toBeUndefined();
    });
  });

  it("shows an error message when selected node properties cannot be serialized", async () => {
    mockUl.isAvailable = true;
    mockUl.getAppVersion.mockReturnValueOnce("3.1.4");
    const Root = await loadRoot();
    const store = createStore();

    const badProps: FBXNodeProps = {
      name: "Bad node",
      properties: [{ name: "BigInt", type: "custom", value: 10n as any }],
      attributes: [],
    };
    store.dispatch({ type: "TREE/NODE_SELECTED", payload: { id: 1 } });
    store.dispatch({ type: "TREE/PROPERTIES_LOADED", payload: { id: 1, properties: badProps } });

    render(
      <Provider store={store}>
        <Root />
      </Provider>
    );

    const textarea = screen.getByTestId("properties-textarea") as HTMLTextAreaElement;
    expect(textarea.value).toContain("Error serializing properties");
    expect(textarea.value).toMatch(/BigInt/i);
  });
});
