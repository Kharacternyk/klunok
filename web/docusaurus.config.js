const prism = require("prism-react-renderer");

/** @type {import('@docusaurus/types').Config} */
const config = {
  title: "Klunok",
  url: "https://klunok.org/",
  favicon: "/favicon.ico",
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
          customCss: "./custom.css",
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
    colorMode: {
      respectPrefersColorScheme: true,
    },
    prism: {
      theme: prism.themes.okaidia,
      additionalLanguages: ["lua"],
    },
    algolia: {
      appId: "S86UJLB8SZ",
      apiKey: "fbaa1ec90ff857f010dc66e07adfef1f",
      indexName: "klunok",
    },
  },
  themes: ["@docusaurus/theme-mermaid"],
  markdown: {
    mermaid: true,
  },
};

module.exports = config;
