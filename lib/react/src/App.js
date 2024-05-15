import bkgImage1 from "/assets/test1.png";
import bkgImage2 from "/assets/test2.png";
import { useRef, useEffect, useState } from 'react';

export function App() {
  const [img, setImg] = useState(null);

  useEffect(() => {
    buttonRef.current.focus();
    setImg(bkgImage1);
  }, []);

  // See the [1] about how to set focus after rendering.
  // [1] https://tigeroakes.com/posts/react-focus-on-render/
  const buttonRef = useRef(null);


  let onKeyPressed = (e) => {
    console.log(e.key);
    setImg(bkgImage2);
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
