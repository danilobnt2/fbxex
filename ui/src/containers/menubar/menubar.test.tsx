import React from "react";
import { describe, expect, it, vi } from "vitest";
import { render, screen } from "@testing-library/react";
import { Provider } from "react-redux";
import { AnyAction, configureStore } from "@reduxjs/toolkit";

import MenuBar, { IPlatformUtils } from "./menubar";

const renderWithStore = (ui: React.ReactElement) => {
  const actions: AnyAction[] = [];
  const store = configureStore({
    reducer: (state = {}, action) => {
      actions.push(action);
      return state;
    },
    preloadedState: {},
  });
  actions.length = 0;

  const result = render(<Provider store={store}>{ui}</Provider>);
  return { ...result, actions };
};

const createPlatform = (overrides: Partial<IPlatformUtils> = {}): IPlatformUtils => ({
  isPlatformAvailable: () => true,
  selectFbxFile: vi.fn().mockReturnValue(true),
  closeWindow: vi.fn(),
  openAboutDialog: vi.fn(),
  createFileSelectedAction: () => ({ type: "FILE_SELECTED" }),
  onPlatformNotAvailable: vi.fn(),
  ...overrides,
});

describe("MenuBar", () => {
  it("dispatches file selected action when open succeeds", () => {
    const platform = createPlatform();
    const { actions } = renderWithStore(<MenuBar {...platform} />);

    screen.getAllByTestId("menu-item-open")[0].click();

    expect(platform.selectFbxFile).toHaveBeenCalled();
    expect(actions.some((a) => a.type === "FILE_SELECTED")).toBe(true);
  });

  it("does not dispatch when file selection is cancelled", () => {
    const platform = createPlatform({ selectFbxFile: vi.fn().mockReturnValue(false) });
    const { actions } = renderWithStore(<MenuBar {...platform} />);

    screen.getAllByTestId("menu-item-open")[0].click();

    expect(platform.selectFbxFile).toHaveBeenCalled();
    const dispatched = actions.filter((a) => a.type !== "@@INIT");
    expect(dispatched).toHaveLength(0);
  });

  it("calls onPlatformNotAvailable when opening while unavailable", () => {
    const platform = createPlatform({ isPlatformAvailable: () => false });
    renderWithStore(<MenuBar {...platform} />);

    screen.getAllByTestId("menu-item-open")[0].click();

    expect(platform.onPlatformNotAvailable).toHaveBeenCalled();
    expect(platform.selectFbxFile).not.toHaveBeenCalled();
  });

  it("closes window when requested", () => {
    const platform = createPlatform();
    renderWithStore(<MenuBar {...platform} />);

    screen.getAllByTestId("menu-item-close")[0].click();

    expect(platform.closeWindow).toHaveBeenCalled();
  });

  it("shows platform not available when closing without availability", () => {
    const platform = createPlatform({ isPlatformAvailable: () => false });
    renderWithStore(<MenuBar {...platform} />);

    screen.getAllByTestId("menu-item-close")[0].click();

    expect(platform.onPlatformNotAvailable).toHaveBeenCalled();
    expect(platform.closeWindow).not.toHaveBeenCalled();
  });

  it("opens about dialog from Help menu", () => {
    const platform = createPlatform();
    renderWithStore(<MenuBar {...platform} />);

    screen.getAllByTestId("menu-item-about")[0].click();

    expect(platform.openAboutDialog).toHaveBeenCalled();
  });

  it("handles unavailable platform when opening about dialog", () => {
    const platform = createPlatform({ isPlatformAvailable: () => false });
    renderWithStore(<MenuBar {...platform} />);

    screen.getAllByTestId("menu-item-about")[0].click();

    expect(platform.onPlatformNotAvailable).toHaveBeenCalled();
    expect(platform.openAboutDialog).not.toHaveBeenCalled();
  });
});
