import { configureStore, Middleware } from "@reduxjs/toolkit";
import {
  TreeAwareState,
  TreeViewAction,
  requestExpandNode,
  resetTree,
  createTreeViewMiddleware,
  treeViewReducer,
  ITreePlatform,
} from "./viewmodels/treeview";

type FileSelectedAction = {
  type: "FILE_SELECTED";
  payload: { format: string | null };
};

type AppStateShape = TreeAwareState & { loadedFileFormat: string | null };

type Actions = TreeViewAction | FileSelectedAction;

const rootReducer = (state: AppStateShape | undefined, action: Actions): AppStateShape => ({
  tree: treeViewReducer(state?.tree, action),
  loadedFileFormat:
    (action as FileSelectedAction).type === "FILE_SELECTED"
      ? (action as FileSelectedAction).payload.format
      : (state?.loadedFileFormat ?? null),
});

export const fileSelected = (format: string | null): FileSelectedAction => ({
  type: "FILE_SELECTED",
  payload: { format },
});

const fileSelectedMiddleware: Middleware<{}, AppStateShape> = (storeApi) => (next) => (action) => {
  const result = next(action);
  if ((action as FileSelectedAction).type === "FILE_SELECTED") {
    storeApi.dispatch(resetTree());
    storeApi.dispatch(requestExpandNode(storeApi.getState().tree.rootId));
  }
  return result;
};

export const createAppStore = (platform: ITreePlatform) =>
  configureStore({
    reducer: rootReducer,
    middleware: (getDefaultMiddleware) =>
      getDefaultMiddleware().concat(createTreeViewMiddleware(platform), fileSelectedMiddleware),
  });

export type AppStore = ReturnType<typeof createAppStore>;
export type AppDispatch = AppStore["dispatch"];
export type AppState = ReturnType<AppStore["getState"]>;
