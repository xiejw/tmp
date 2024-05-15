import bkgImage from "/assets/test.jpg";
import { useRef, useEffect, useState } from 'react';

export function App() {
  // See the [1] about how to set focus after rendering.
  // [1] https://tigeroakes.com/posts/react-focus-on-render/
  const buttonRef = useRef(null);

  useEffect(() => {
    buttonRef.current.focus();
  }, []);

  const [img, setImg] = useState(null);

  let onKeyPressed = (e) => {
    console.log(e.key);
    setImg(bkgImage);
  }

  return <div
      className="img_viewer"
      style={{
        backgroundImage: `url(${img})`
      }}
      tabIndex="0"
      onKeyDown={onKeyPressed}
      ref={buttonRef}
    >
    </div>;
}
