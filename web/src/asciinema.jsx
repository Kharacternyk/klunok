import BrowserOnly from "@docusaurus/BrowserOnly";
import "asciinema-player/dist/bundle/asciinema-player.css";
import React, {useEffect, useRef, useState} from "react";

export default ({ src }) => (
  <BrowserOnly fallback={"terminal screencast"}>
    {() => {
      const ref = useRef();
      const [player, setPlayer] = useState();

      useEffect(() => import("asciinema-player").then(setPlayer), []);
      useEffect(() => {
        if (player) {
          const instance = player.create(src, ref.current, {
            idleTimeLimit: 3,
            autoPlay: true,
            loop: true,
          });
          return () => instance.dispose();
        }
      }, [src, player]);

      return <div ref={ref}></div>;
    }}
  </BrowserOnly>
);
