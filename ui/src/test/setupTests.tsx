import React from "react";
import { afterEach, expect, vi } from "vitest";
import * as matchers from "@testing-library/jest-dom/matchers";
import { cleanup } from "@testing-library/react";

expect.extend(matchers);

afterEach(() => {
  cleanup();
  vi.clearAllMocks();
});

vi.mock("@heroui/button", () => ({
  Button: ({
    children,
    onPress,
    disableRipple: _disableRipple,
    isIconOnly: _isIconOnly,
    isDisabled: _isDisabled,
    startContent: _startContent,
    ...rest
  }: any) => (
    <button type="button" {...rest} onClick={onPress ?? rest.onClick}>
      {children}
    </button>
  ),
}));

vi.mock("@heroui/dropdown", () => {
  const Dropdown = ({ children }: any) => <div>{children}</div>;
  const DropdownTrigger = ({ children }: any) => <div>{children}</div>;
  const DropdownMenu = ({ children, onAction, itemClasses: _itemClasses, ...rest }: any) => {
    const items = React.Children.toArray(children) as any[];
    return (
      <div role="menu" {...rest}>
        {items.map((child, index) => {
          const rawKey = child?.props?.itemKey ?? child?.key ?? index;
          const actionKey = typeof rawKey === "string" ? rawKey.replace(/^[.$]+/, "") : String(rawKey);
          return React.cloneElement(child, {
            key: index,
            itemKey: actionKey,
            onClick: () => {
              child?.props?.onClick?.();
              onAction?.(actionKey);
            },
          });
        })}
      </div>
    );
  };
  const DropdownItem = ({ children, onClick, itemKey, ...rest }: any) => (
    <button
      type="button"
      data-testid={typeof itemKey === "string" ? `menu-item-${itemKey}` : undefined}
      onClick={onClick}
      {...rest}
    >
      {children}
    </button>
  );
  return { Dropdown, DropdownTrigger, DropdownMenu, DropdownItem };
});

vi.mock("@heroui/navbar", () => {
  const Navbar = ({ children, classNames: _classNames, maxWidth: _maxWidth, ...rest }: any) => <nav {...rest}>{children}</nav>;
  const NavbarContent = ({ children, classNames: _classNames, ...rest }: any) => <div {...rest}>{children}</div>;
  const NavbarItem = ({ children, classNames: _classNames, ...rest }: any) => <div {...rest}>{children}</div>;
  return { Navbar, NavbarContent, NavbarItem };
});

vi.mock("@heroui/card", () => {
  const Card = ({ children, ...props }: any) => <div {...props}>{children}</div>;
  const CardBody = ({ children, ...props }: any) => <div {...props}>{children}</div>;
  const CardHeader = ({ children, ...props }: any) => <div {...props}>{children}</div>;
  return { Card, CardBody, CardHeader };
});

vi.mock("@heroui/chip", () => ({
  Chip: ({ children, startContent: _startContent, ...props }: any) => <span {...props}>{children}</span>,
}));

vi.mock("@heroui/scroll-shadow", () => ({
  ScrollShadow: ({ children, ...props }: any) => <div {...props}>{children}</div>,
}));

vi.mock("@heroui/spinner", () => ({
  Spinner: ({ children, ...props }: any) => <span {...props}>{children ?? "Loading"}</span>,
}));
