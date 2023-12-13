const prism = require("prism-react-renderer");

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
  },
  themes: ["@docusaurus/theme-mermaid"],
  markdown: {
    mermaid: true,
  },
};

module.exports = config;
