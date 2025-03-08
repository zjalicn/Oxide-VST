# Web Component Styling

The initial goal was to create re-usable web components that could be used in the OXIDE app. Eventually I'd like to get React support potentially if the state management gets crazy. But for now I just wanted a simple way to start seperating out my code.

I want to make easily interchangeable themes so I can re-use a lot of the same styling for different components and swap them out in the future. This made SCSS a logical choice since it's lightweight and I don't need the full strength of react yet (if ever). Eventually I'm thinking maybe making some tailwind styles might be the move.

As a workaround currently I'm using a placeholder global scss import that gets replaced with the actual styles when the app is built. As a result the compile_scss file needs to be updated for each new component
