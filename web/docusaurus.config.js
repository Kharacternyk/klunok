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
        theme: {
          customCss: [require.resolve("./css/custom.css")],
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
      logo: {
        src: "logo.svg",
      },
    },
    prism: {
      theme: codeTheme,
      additionalLanguages: ["lua"],
    },
  },
  themes: ["@docusaurus/theme-mermaid"],
  markdown: {
    mermaid: true,
  },
};

module.exports = config;
