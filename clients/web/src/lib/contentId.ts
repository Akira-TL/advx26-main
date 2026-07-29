const CONTENT_ID_PATTERN = /[0-9a-f]{32}/i;

export function extractContentId(input: string | null | undefined): string {
  const match = String(input || "")
    .toLowerCase()
    .match(CONTENT_ID_PATTERN);

  return match?.[0] || "";
}

export function getInitialContentId(): string {
  const params = new URLSearchParams(window.location.search);

  return (
    extractContentId(params.get("id")) ||
    extractContentId(window.location.hash) ||
    extractContentId(window.location.href)
  );
}
