import React from "react";
import "./menubar.css";

import ul from "../../ul";

type MenuId = "file" | "help";
type DropId = "open" | "close" | "about";

function classNames(...parts: Array<string | false | null | undefined>) {
  return parts.filter(Boolean).join(" ");
}

function useMenuState() {
  const [openMenu, setOpenMenu] = React.useState<MenuId | null>(null);
  const [hoveredMenu, setHoveredMenu] = React.useState<MenuId | null>(null);
  const [hoveredDrop, setHoveredDrop] = React.useState<DropId | null>(null);
  const rootRef = React.useRef<HTMLDivElement | null>(null);

  React.useEffect(() => {
    function onDocClick(e: MouseEvent) {
      if (!rootRef.current?.contains(e.target as Node)) setOpenMenu(null);
    }
    document.addEventListener("click", onDocClick);
    return () => document.removeEventListener("click", onDocClick);
  }, []);

  return {
    rootRef,
    openMenu,
    hoveredMenu,
    hoveredDrop,
    toggleMenu: (id: MenuId) => setOpenMenu((m) => (m === id ? null : id)),
    setHoveredMenu,
    setHoveredDrop,
    closeMenu: () => setOpenMenu(null),
  };
}

type MenuStateContextValue = {
  openMenu: MenuId | null;
  hoveredMenu: MenuId | null;
  toggleMenu: (id: MenuId) => void;
  setHoveredMenu: (id: MenuId | null) => void;
};

const MenuStateContext = React.createContext<MenuStateContextValue | null>(null);

function useMenuStateContext() {
  const ctx = React.useContext(MenuStateContext);
  if (!ctx) throw new Error("MenuStateContext used outside provider");
  return ctx;
}

type MenuContextValue = {
  hoveredDrop: DropId | null;
  setHoveredDrop: React.Dispatch<React.SetStateAction<DropId | null>>;
  closeMenu: () => void;
};

const MenuContext = React.createContext<MenuContextValue | null>(null);

function useMenuContext() {
  const ctx = React.useContext(MenuContext);
  if (!ctx) throw new Error("MenuContext used outside provider");
  return ctx;
}

type MenuProps = {
  id: MenuId;
  label: string;
  children: React.ReactNode;
};

function Menu({ id, label, children }: MenuProps) {
  const { openMenu, hoveredMenu, toggleMenu, setHoveredMenu } = useMenuStateContext();
  const isOpen = openMenu === id;
  const isHovered = hoveredMenu === id;

  return (
    <div
      className={classNames("menu-item", isHovered && "is-hovered", isOpen && "is-active")}
      onClick={(e) => {
        e.stopPropagation();
        toggleMenu(id);
      }}
      onMouseEnter={() => setHoveredMenu(id)}
      onMouseLeave={() => setHoveredMenu(null)}
    >
      {label}
      {isOpen && <div className="menu-dropdown" role="menu">{children}</div>}
    </div>
  );
}

type DropProps = {
  id: DropId;
  label: string;
  onClick: () => void;
};

function Drop({ id, label, onClick }: DropProps) {
  const { hoveredDrop, closeMenu, setHoveredDrop } = useMenuContext();
  const isHovered = hoveredDrop === id;

  return (
    <div
      className={classNames("drop-item", isHovered && "is-hovered")}
      onClick={(e) => {
        e.stopPropagation();
        closeMenu();
        onClick();
      }}
      onMouseEnter={() => setHoveredDrop(id)}
      onMouseLeave={() => setHoveredDrop(null)}
    >
      {label}
    </div>
  );
}

export default function MenuBar() {
  const {
    rootRef,
    openMenu,
    hoveredMenu,
    hoveredDrop,
    toggleMenu,
    setHoveredMenu,
    setHoveredDrop,
    closeMenu,
  } = useMenuState();

  return (
    <MenuStateContext.Provider
      value={{ openMenu, hoveredMenu, toggleMenu, setHoveredMenu }}
    >
      <MenuContext.Provider value={{ hoveredDrop, setHoveredDrop, closeMenu }}>
        <div ref={rootRef} className="menu-bar">
          
          <Menu id="file" label="File">
            <Drop 
              id="open" 
              label="Open ..." 
              onClick={() => ul.isAvailable ? ul.selectFbxFile() : alert("Ultralight not available.")}
            />
            <Drop 
              id="close" 
              label="Close" 
              onClick={() => ul.isAvailable ? ul.closeWindow() : alert("Ultralight not available.")}
            />
          </Menu>

          <Menu id="help" label="Help">
            <Drop
              id="about"
              label="About"
              onClick={() => ul.isAvailable ? ul.openAboutDialog() : alert("Ultralight not available.")}
            />
          </Menu>
        </div>
      </MenuContext.Provider>
    </MenuStateContext.Provider>
  );
}
