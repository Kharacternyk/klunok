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
  themeConfig: {
    navbar: {
      title: "Klunok",
      logo: {
        src: "logo.svg",
      },
      items: [
        {
          href: "https://github.com/Kharacternyk/klunok",
          label: "GitHub",
          position: "right",
        },
      ],
    },
    footer: {
      copyright:
        '&copy; 2022&ndash;2026, Nazar Vinnichuk (<a href="https://vinnich.uk">https://vinnich.uk</a>)',
    },
    colorMode: {
      respectPrefersColorScheme: true,
    },
    prism: {
      theme: prism.themes.okaidia,
      additionalLanguages: [
        "lua",
      ],
    },
    algolia: {
      appId: "S86UJLB8SZ",
      apiKey: "fbaa1ec90ff857f010dc66e07adfef1f",
      indexName: "klunok",
    },
  },
  themes: [
    "@docusaurus/theme-mermaid",
  ],
  markdown: {
    mermaid: true,
  },
};

module.exports = config;
