import React from "react";

export default ({ children }) => {
  const isUnreleased = children === undefined;
  const className = `badge badge--${isUnreleased ? "danger" : "primary"}`;
  const label = isUnreleased ? "unreleased" : `≥ v${children}`;

  return <span className={className}>{label}</span>;
};
