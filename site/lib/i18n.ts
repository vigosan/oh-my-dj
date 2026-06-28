export type Lang = "en" | "es";

export const LANGS: Lang[] = ["en", "es"];

export const SITE_URL = "https://oh-my-dj.vicent.io";
export const REPO_URL = "https://github.com/vigosan/oh-my-dj";
export const RELEASES_URL = "https://github.com/vigosan/oh-my-dj/releases";

type Meta = {
  title: string;
  description: string;
  keywords: string[];
};

export const meta: Record<Lang, Meta> = {
  en: {
    title: "Oh My DJ — OBS multistream & multi-camera plugin for DJs",
    description:
      "Free OBS Studio plugin for DJs: multistream to Twitch, YouTube and Facebook at once, and rotate multiple cameras on a timer — automatically. A lighter alternative to obs-multi-rtmp + Advanced Scene Switcher.",
    keywords: [
      "OBS multistream plugin",
      "OBS multistream",
      "multistream OBS",
      "stream to Twitch and YouTube at the same time",
      "multi camera OBS",
      "multi cam OBS plugin",
      "OBS scene switcher",
      "automatic scene switcher OBS",
      "rotate cameras OBS",
      "OBS camera rotation timer",
      "DJ live stream OBS",
      "OBS plugin for DJs",
      "OBS Studio plugin",
      "OBS Studio dock",
      "OBS dock plugin",
      "OBS addon",
      "best OBS plugins for streaming",
      "obs-multi-rtmp alternative",
      "Advanced Scene Switcher alternative",
      "multistream Twitch YouTube Facebook",
    ],
  },
  es: {
    title: "Oh My DJ — Plugin de OBS multistream y multicámara para DJs",
    description:
      "Plugin gratuito de OBS Studio para DJs: emite a Twitch, YouTube y Facebook a la vez y rota varias cámaras con temporizador — automáticamente. Alternativa ligera a obs-multi-rtmp + Advanced Scene Switcher.",
    keywords: [
      "plugin OBS multistream",
      "multistream OBS",
      "emitir a Twitch y YouTube a la vez",
      "multicámara OBS",
      "multi cam OBS",
      "cambio de escenas OBS",
      "cambio automático de escenas OBS",
      "rotar cámaras OBS",
      "rotación de cámaras OBS temporizador",
      "directo DJ OBS",
      "plugin OBS para DJs",
      "plugin OBS Studio",
      "complemento OBS Studio",
      "addon OBS",
      "dock OBS",
      "mejores plugins OBS para streaming",
      "alternativa obs-multi-rtmp",
      "alternativa Advanced Scene Switcher",
      "multistream Twitch YouTube Facebook",
    ],
  },
};

type Feature = {
  tag: string;
  title: string;
  body: string;
  rows: string[][];
  cols: string[];
};

type Step = { n: string; title: string; body: string };

export type Dict = {
  nav: { features: string; how: string; faq: string; download: string; github: string };
  langLabel: string;
  hero: {
    badge: string;
    title: string;
    titleAccent: string;
    subtitle: string;
    download: string;
    source: string;
    replaces: string;
  };
  dock: {
    title: string;
    cameras: string;
    streaming: string;
    showing: string;
    next: string;
    sync: string;
    live: string;
  };
  pitch: { tag: string; title: string; body: string };
  featuresTitle: string;
  rotation: Feature;
  multistream: Feature;
  howTitle: string;
  howTag: string;
  steps: Step[];
  download: {
    tag: string;
    title: string;
    body: string;
    note: string;
    downloads: string;
  };
  faqTag: string;
  faqTitle: string;
  faq: { q: string; a: string }[];
  footer: { tagline: string; donate: string; license: string; built: string };
};

export const dict: Record<Lang, Dict> = {
  en: {
    nav: { features: "Features", how: "How it works", faq: "FAQ", download: "Download", github: "GitHub" },
    langLabel: "ES",
    hero: {
      badge: "OBS Studio plugin",
      title: "One dock for DJs.",
      titleAccent: "Rotate cameras. Multistream. Automatically.",
      subtitle:
        "Oh My DJ is an OBS plugin that rotates your scenes on a timer and pushes your stream to several platforms at once — all from a single, focused panel. No manual Start/Stop. It just follows OBS.",
      download: "Download",
      source: "View source",
      replaces:
        "Replaces the heavyweight, untranslated setup of obs-multi-rtmp + Advanced Scene Switcher.",
    },
    dock: {
      title: "Oh My DJ — Live",
      cameras: "Cameras",
      streaming: "Streaming",
      showing: "Showing",
      next: "next",
      sync: "Synced with OBS",
      live: "LIVE",
    },
    pitch: {
      tag: "What it is",
      title: "One focused panel instead of two heavy ones.",
      body: "DJs shouldn't fight with OBS automation tools. Oh My DJ collapses scene switching and multistreaming into a single dock with two tabs — Cameras and Streaming. Both features are fully automatic: you only enable them, and the plugin reacts to OBS events on its own.",
    },
    featuresTitle: "Two tabs. That's the whole plugin.",
    rotation: {
      tag: "Cameras",
      title: "Scene rotation on a timer",
      body: "Define a flow of scenes, each shown for a chosen amount of seconds, minutes or hours, then loop. Tick Automatic rotation and the flow engages on its own whenever the program scene matches a step. Per step you can override what happens when time ends: advance to the next scene, or jump to a specific one (e.g. always return to your main camera).",
      cols: ["Scene", "Duration", "Unit", "When it ends"],
      rows: [
        ["CAM 1", "2", "minutes", "Next scene (loop)"],
        ["CAM 2", "30", "seconds", "Next scene (loop)"],
        ["CAM 3", "1", "minutes", "→ CAM 1"],
      ],
    },
    multistream: {
      tag: "Streaming",
      title: "Multistream to every platform",
      body: "Pick a platform — Twitch, YouTube, Facebook Live… — and its RTMP server URL is filled in for you. Paste the stream key, tick it On, and enable Sync with OBS streaming. Now every enabled destination goes live when you press Start Streaming in OBS, reusing OBS's own encoder — same resolution and bitrate, zero extra CPU — and stops when you stop it.",
      cols: ["Platform", "Server (RTMP)", "On", "Status"],
      rows: [
        ["Twitch", "rtmp://live.twitch.tv/app", "on", "live"],
        ["YouTube", "rtmp://a.rtmp.youtube.com/live2", "on", "live"],
        ["Facebook", "rtmps://live-api-s.facebook.com", "off", "idle"],
      ],
    },
    howTitle: "Automatic by design",
    howTag: "How it works",
    steps: [
      {
        n: "01",
        title: "Enable, don't operate",
        body: "There are no manual Start/Stop buttons. You set up your flow and your destinations once, tick the enable boxes, and forget about it.",
      },
      {
        n: "02",
        title: "It follows OBS",
        body: "Rotation runs only while OBS is streaming. Multistream destinations go live when you press Start Streaming in OBS and stop when you stop it.",
      },
      {
        n: "03",
        title: "Saved per profile",
        body: "Your scene flow and stream destinations are stored per OBS profile, so different gigs keep their own setup.",
      },
    ],
    download: {
      tag: "Download",
      title: "Get Oh My DJ",
      body: "Pick your platform, drop the plugin into your OBS plugins folder, and restart OBS.",
      note: "Links to the latest GitHub Release. Prefer to build it yourself? The source is on GitHub.",
      downloads: "downloads",
    },
    faqTag: "FAQ",
    faqTitle: "Questions about the OBS multicam & multistream plugin",
    faq: [
      {
        q: "What is the best OBS multicam plugin for DJs?",
        a: "Oh My DJ is a free OBS Studio plugin built specifically for DJs. It rotates multiple cameras (scenes) on a timer and multistreams to several platforms at once, from a single dock with two tabs. It's a lighter alternative to combining obs-multi-rtmp with Advanced Scene Switcher.",
      },
      {
        q: "How do I stream to Twitch and YouTube at the same time in OBS?",
        a: "Install Oh My DJ, open the Streaming tab, pick Twitch and YouTube (their RTMP server URLs fill in automatically), paste each stream key, tick them On and enable Sync with OBS streaming. When you press Start Streaming in OBS, both platforms go live at once, reusing OBS's own encoder — no extra CPU.",
      },
      {
        q: "How do I rotate multiple cameras automatically in OBS?",
        a: "In the Cameras tab, define a flow of scenes and how long each one shows (seconds, minutes or hours), then tick Automatic rotation. When the program scene matches a step, Oh My DJ advances on schedule and loops — no manual switching, no extra clicks.",
      },
      {
        q: "Is Oh My DJ a good obs-multi-rtmp alternative?",
        a: "Yes. obs-multi-rtmp handles multistreaming and Advanced Scene Switcher handles scene automation — Oh My DJ does both from one focused, translated panel designed for DJs, with no manual Start/Stop. It follows OBS automatically.",
      },
      {
        q: "Is Oh My DJ free, and what platforms does it support?",
        a: "It's free and open source (GPL-2.0). It runs on macOS, Windows and Linux, and requires OBS Studio. Download the plugin for your platform, drop it into your OBS plugins folder and restart OBS.",
      },
    ],
    footer: {
      tagline: "An OBS plugin that makes life simple for DJs.",
      donate: "Donate ♥",
      license: "Open source",
      built: "Built with OBS, Qt6 and CMake.",
    },
  },
  es: {
    nav: { features: "Funciones", how: "Cómo funciona", faq: "FAQ", download: "Descarga", github: "GitHub" },
    langLabel: "EN",
    hero: {
      badge: "Plugin de OBS Studio",
      title: "Un solo panel para DJs.",
      titleAccent: "Rota cámaras. Multistream. Automático.",
      subtitle:
        "Oh My DJ es un plugin de OBS que rota tus escenas con un temporizador y emite tu directo a varias plataformas a la vez — todo desde un único panel. Sin botones de Start/Stop manuales. Simplemente sigue a OBS.",
      download: "Descargar",
      source: "Ver código",
      replaces:
        "Sustituye la configuración pesada y sin traducir de obs-multi-rtmp + Advanced Scene Switcher.",
    },
    dock: {
      title: "Oh My DJ — En directo",
      cameras: "Cámaras",
      streaming: "Streaming",
      showing: "Mostrando",
      next: "siguiente",
      sync: "Sincronizado con OBS",
      live: "EN VIVO",
    },
    pitch: {
      tag: "Qué es",
      title: "Un único panel en lugar de dos pesados.",
      body: "Un DJ no debería pelearse con las herramientas de automatización de OBS. Oh My DJ reúne el cambio de escenas y el multistreaming en un solo dock con dos pestañas — Cámaras y Streaming. Ambas funciones son totalmente automáticas: solo las activas y el plugin reacciona a los eventos de OBS por su cuenta.",
    },
    featuresTitle: "Dos pestañas. Eso es todo el plugin.",
    rotation: {
      tag: "Cámaras",
      title: "Rotación de escenas con temporizador",
      body: "Define un flujo de escenas, cada una mostrada durante los segundos, minutos u horas que elijas, y vuelve a empezar. Activa Rotación automática y el flujo se pone en marcha solo cuando la escena de programa coincide con un paso. En cada paso puedes decidir qué pasa al acabar el tiempo: avanzar a la siguiente o saltar a una escena concreta (p. ej. volver siempre a tu cámara principal).",
      cols: ["Escena", "Duración", "Unidad", "Al terminar"],
      rows: [
        ["CAM 1", "2", "minutos", "Siguiente (bucle)"],
        ["CAM 2", "30", "segundos", "Siguiente (bucle)"],
        ["CAM 3", "1", "minutos", "→ CAM 1"],
      ],
    },
    multistream: {
      tag: "Streaming",
      title: "Multistream a cada plataforma",
      body: "Elige una plataforma — Twitch, YouTube, Facebook Live… — y se rellena su URL de servidor RTMP. Pega la clave de stream, márcala como activa y activa Sincronizar con el streaming de OBS. Ahora cada destino activo se pone en directo al pulsar Iniciar transmisión en OBS, reutilizando el propio codificador de OBS — misma resolución y bitrate, cero CPU extra — y se detiene cuando lo paras.",
      cols: ["Plataforma", "Servidor (RTMP)", "Activo", "Estado"],
      rows: [
        ["Twitch", "rtmp://live.twitch.tv/app", "on", "live"],
        ["YouTube", "rtmp://a.rtmp.youtube.com/live2", "on", "live"],
        ["Facebook", "rtmps://live-api-s.facebook.com", "off", "idle"],
      ],
    },
    howTitle: "Automático por diseño",
    howTag: "Cómo funciona",
    steps: [
      {
        n: "01",
        title: "Activar, no operar",
        body: "No hay botones de Start/Stop manuales. Configuras tu flujo y tus destinos una vez, marcas las casillas de activación y te olvidas.",
      },
      {
        n: "02",
        title: "Sigue a OBS",
        body: "La rotación solo corre mientras OBS está emitiendo. Los destinos de multistream se ponen en directo al pulsar Iniciar transmisión en OBS y se paran cuando lo detienes.",
      },
      {
        n: "03",
        title: "Guardado por perfil",
        body: "Tu flujo de escenas y tus destinos se guardan por perfil de OBS, así cada bolo mantiene su propia configuración.",
      },
    ],
    download: {
      tag: "Descarga",
      title: "Consigue Oh My DJ",
      body: "Elige tu plataforma, copia el plugin en tu carpeta de plugins de OBS y reinicia OBS.",
      note: "Enlaza a la última release de GitHub. ¿Prefieres compilarlo tú? El código está en GitHub.",
      downloads: "descargas",
    },
    faqTag: "FAQ",
    faqTitle: "Preguntas sobre el plugin de OBS multicámara y multistream",
    faq: [
      {
        q: "¿Cuál es el mejor plugin de OBS multicámara para DJs?",
        a: "Oh My DJ es un plugin gratuito de OBS Studio pensado para DJs. Rota varias cámaras (escenas) con un temporizador y emite a varias plataformas a la vez, desde un único dock con dos pestañas. Es una alternativa más ligera a combinar obs-multi-rtmp con Advanced Scene Switcher.",
      },
      {
        q: "¿Cómo emito a Twitch y YouTube a la vez en OBS?",
        a: "Instala Oh My DJ, abre la pestaña Streaming, elige Twitch y YouTube (sus URLs de servidor RTMP se rellenan solas), pega cada clave de stream, márcalas como activas y activa Sincronizar con el streaming de OBS. Al pulsar Iniciar transmisión en OBS, ambas plataformas se ponen en directo a la vez, reutilizando el codificador de OBS — sin CPU extra.",
      },
      {
        q: "¿Cómo roto varias cámaras automáticamente en OBS?",
        a: "En la pestaña Cámaras, define un flujo de escenas y cuánto se muestra cada una (segundos, minutos u horas) y activa Rotación automática. Cuando la escena de programa coincide con un paso, Oh My DJ avanza según el horario y vuelve a empezar — sin cambios manuales ni clics extra.",
      },
      {
        q: "¿Es Oh My DJ una buena alternativa a obs-multi-rtmp?",
        a: "Sí. obs-multi-rtmp se encarga del multistreaming y Advanced Scene Switcher de la automatización de escenas — Oh My DJ hace ambas cosas desde un único panel, traducido y pensado para DJs, sin Start/Stop manuales. Sigue a OBS automáticamente.",
      },
      {
        q: "¿Es gratis Oh My DJ y qué plataformas soporta?",
        a: "Es gratuito y de código abierto (GPL-2.0). Funciona en macOS, Windows y Linux, y requiere OBS Studio. Descarga el plugin para tu plataforma, cópialo en tu carpeta de plugins de OBS y reinicia OBS.",
      },
    ],
    footer: {
      tagline: "Un plugin de OBS que le hace la vida fácil a los DJs.",
      donate: "Donar ♥",
      license: "Código abierto",
      built: "Hecho con OBS, Qt6 y CMake.",
    },
  },
};
