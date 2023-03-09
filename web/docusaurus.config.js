const codeTheme = require("prism-react-renderer/themes/okaidia");

const config = {
  title: "Klunok",
  // TODO production url
  url: "https://your-docusaurus-test-site.com",
  baseUrl: "/",
  presets: [
    [
      "classic",
      {
        docs: {
          routeBasePath: "/",
        },
      },
    ],
  ],
  plugins: [
    () => ({
      configureWebpack: () => ({
        resolve: {
          symlinks: false,
        },
      }),
    }),
  ],

  themeConfig: {
    navbar: {
      title: "Klunok",
      //TODO Logo
    },
    prism: {
      theme: codeTheme,
      additionalLanguages: ["lua"],
    },
  },
};

module.exports = config;
