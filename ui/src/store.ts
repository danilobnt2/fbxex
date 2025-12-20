import { configureStore, Middleware } from "@reduxjs/toolkit";
import ul from "./ul";

type RootState = {
  rootChildren: number[] | null;
};

const initialState: RootState = {
  rootChildren: null,
};

type SetRootChildrenAction = {
  type: "SET_ROOT_CHILDREN";
  payload: number[] | null;
};

type FileSelectedAction = {
  type: "FILE_SELECTED";
};

type Actions = SetRootChildrenAction | FileSelectedAction;

const reducer = (state: RootState = initialState, action: Actions): RootState => {
  switch (action.type) {
    case "SET_ROOT_CHILDREN":
      return { ...state, rootChildren: action.payload };
    case "FILE_SELECTED":
      return state;
    default:
      return state;
  }
};

export const setRootChildren = (children: number[] | null): SetRootChildrenAction => ({
  type: "SET_ROOT_CHILDREN",
  payload: children,
});

export const fileSelected = (): FileSelectedAction => ({
  type: "FILE_SELECTED",
});

const fileSelectedMiddleware: Middleware<{}, RootState> = (storeApi) => (next) => (action) => {
  const result = next(action);
  if ((action as FileSelectedAction).type === "FILE_SELECTED") {
    if (!ul.isAvailable) {
      storeApi.dispatch(setRootChildren(null));
      return result;
    }
    try {
      const children = ul.getFBXNodeChildren(0);
      storeApi.dispatch(setRootChildren(children));
    } catch (err) {
      console.error(err);
      storeApi.dispatch(setRootChildren(null));
    }
  }
  return result;
};

export const store = configureStore({
  reducer,
  middleware: (getDefaultMiddleware) => getDefaultMiddleware().concat(fileSelectedMiddleware),
});

export type AppDispatch = typeof store.dispatch;
export type AppState = ReturnType<typeof store.getState>;
