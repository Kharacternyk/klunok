const codeTheme = require("prism-react-renderer/themes/okaidia");

const config = {
  title: "Klunok",
  url: "https://klunok.org/",
  baseUrl: "/",
  trailingSlash: false,
  presets: [
    [
      "classic",
      {
        docs: {
          routeBasePath: "/",
          breadcrumbs: false,
        },
        theme: {
          customCss: [require.resolve("./css/custom.css")],
        },
      },
    ],
  ],
  plugins: [
    require.resolve("@cmfcmf/docusaurus-search-local"),
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
