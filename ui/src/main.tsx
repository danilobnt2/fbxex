import React from "react";
import ReactDOM from "react-dom/client";

import { HeroUIProvider } from "@heroui/system";
import { Spacer } from "@heroui/spacer";

import "./index.css";

import ul from "./ul";
import { MenuBar } from "./containers";


declare global {
  interface Window {
    __remountApp?: () => void
    __ultralight?: typeof ul
  }
}


window.__ultralight = ul;


const getFbxFileFormatVersion = () => 
  ul.isAvailable ? ul.getFbxFileFormatVersion() : "🤷";


function Root() {
  const [key, setKey] = React.useState(0)

  React.useEffect(() => {
    window.__remountApp = () => setKey(k => k + 1)
    return () => {
      delete window.__remountApp
    }
  }, [])

  return (
    <React.StrictMode key={key}>
      <HeroUIProvider>
        <div className="h-screen flex flex-col">
          <MenuBar />
          <div className="flex-1 flex items-center justify-center">
            <div className="text-center">
              <p className="text-5xl font-bold">fbxex</p>
              <p className="text-2xl">The FBX Explorer and Inspector</p>
              <Spacer y={4} />
              <p>FBX file format version {getFbxFileFormatVersion()}</p>
            </div>
          </div>
        </div>
      </HeroUIProvider>
    </React.StrictMode>
  )
}

ReactDOM.createRoot(document.getElementById("root")!).render(<Root />)
