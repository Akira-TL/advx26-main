const trimTrailingSlash = (value: string) => value.replace(/\/+$/, "");

export const API_BASE = trimTrailingSlash(
  import.meta.env.VITE_API_BASE || "http://soundpola.babelbeast.com:9000",
);

export const APK_URL =
  import.meta.env.VITE_APK_URL || "https://babelbeast.com/app-release.apk";

export const APP_SCHEME = import.meta.env.VITE_APP_SCHEME || "soundpola";
