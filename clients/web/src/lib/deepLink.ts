import { APP_SCHEME } from "./config";

export function getAppContentLink(contentId: string): string {
  return `${APP_SCHEME}://c/${contentId}`;
}
