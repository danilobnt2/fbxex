import { configureStore, Middleware } from "@reduxjs/toolkit";
import {
  TreeAwareState,
  TreeViewAction,
  requestExpandNode,
  resetTree,
  treeViewMiddleware,
  treeViewReducer,
} from "./viewmodels/treeview";

type FileSelectedAction = {
  type: "FILE_SELECTED";
};

type Actions = TreeViewAction | FileSelectedAction;

const rootReducer = (state: TreeAwareState | undefined, action: Actions): TreeAwareState => ({
  tree: treeViewReducer(state?.tree, action),
});

export const fileSelected = (): FileSelectedAction => ({
  type: "FILE_SELECTED",
});

const fileSelectedMiddleware: Middleware<{}, TreeAwareState> = (storeApi) => (next) => (action) => {
  const result = next(action);
  if ((action as FileSelectedAction).type === "FILE_SELECTED") {
    storeApi.dispatch(resetTree());
    storeApi.dispatch(requestExpandNode(storeApi.getState().tree.rootId));
  }
  return result;
};

export const store = configureStore({
  reducer: rootReducer,
  middleware: (getDefaultMiddleware) => getDefaultMiddleware().concat(treeViewMiddleware, fileSelectedMiddleware),
});

export type AppDispatch = typeof store.dispatch;
export type AppState = ReturnType<typeof store.getState>;
