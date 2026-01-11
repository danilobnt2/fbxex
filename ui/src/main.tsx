import ReactDOM from "react-dom/client";

import { Provider } from "react-redux";

import "./main.css";

import ul from "./ul";
import { createAppStore } from "./store";
import { Root } from "./app";
import { UltralightTreePlatform } from "./bindings";


declare global {
  interface Window {
    __remountApp?: () => void
    __ultralight?: typeof ul
  }
}


window.__ultralight = ul;


const treePlatform = new UltralightTreePlatform();
const store = createAppStore(treePlatform);
const rootElement = document.getElementById("root");
if (rootElement) {
  ReactDOM.createRoot(rootElement).render(
    <Provider store={store}>
      <Root />
    </Provider>
  );
}
