export type Lang = "en" | "es";

export const REPO_URL = "https://github.com/vigosan/oh-my-dj";
export const RELEASES_URL = "https://github.com/vigosan/oh-my-dj/releases";

type Feature = {
  tag: string;
  title: string;
  body: string;
  rows: string[][];
  cols: string[];
};

type Step = { n: string; title: string; body: string };

export type Dict = {
  nav: { features: string; how: string; download: string; github: string };
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
  footer: { tagline: string; donate: string; license: string; built: string };
};

export const dict: Record<Lang, Dict> = {
  en: {
    nav: { features: "Features", how: "How it works", download: "Download", github: "GitHub" },
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
    footer: {
      tagline: "An OBS plugin that makes life simple for DJs.",
      donate: "Donate ♥",
      license: "Open source",
      built: "Built with OBS, Qt6 and CMake.",
    },
  },
  es: {
    nav: { features: "Funciones", how: "Cómo funciona", download: "Descarga", github: "GitHub" },
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
    footer: {
      tagline: "Un plugin de OBS que le hace la vida fácil a los DJs.",
      donate: "Donar ♥",
      license: "Código abierto",
      built: "Hecho con OBS, Qt6 y CMake.",
    },
  },
};
