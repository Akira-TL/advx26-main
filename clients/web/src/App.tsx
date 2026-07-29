import * as Tooltip from "@radix-ui/react-tooltip";

import { LocaleProvider } from "./i18n/LocaleContext";
import { extractContentId } from "./lib/contentId";
import { LandingPage } from "./routes/LandingPage";
import { PreviewPage } from "./routes/PreviewPage";

type AppRoute = "landing" | "preview";

function getInitialRoute(): AppRoute {
  const path = window.location.pathname.toLowerCase();
  const params = new URLSearchParams(window.location.search);

  if (
    path.includes("/preview") ||
    path.includes("/c/") ||
    params.has("id") ||
    extractContentId(window.location.hash)
  ) {
    return "preview";
  }

  return "landing";
}

export function App() {
  const route = getInitialRoute();

  return (
    <Tooltip.Provider delayDuration={160}>
      <LocaleProvider>{route === "preview" ? <PreviewPage /> : <LandingPage />}</LocaleProvider>
    </Tooltip.Provider>
  );
}
