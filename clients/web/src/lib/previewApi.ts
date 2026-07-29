import { API_BASE } from "./config";
import type { PreviewAttribute, PreviewContent } from "../types/preview";

interface TokenMetadataResponse {
  name?: string;
  description?: string;
  image?: string;
  attributes?: PreviewAttribute[];
}

export class PreviewApiError extends Error {
  constructor(
    message: string,
    readonly status?: number,
  ) {
    super(message);
    this.name = "PreviewApiError";
  }
}

function fallbackMessage(status: number): string {
  if (status === 404) return "\u5185\u5bb9\u4e0d\u5b58\u5728";
  if (status === 409) return "\u5185\u5bb9\u5904\u7406\u4e2d\uff0c\u8bf7\u7a0d\u540e\u518d\u8bd5";
  return `\u52a0\u8f7d\u5931\u8d25 (${status})`;
}

export async function fetchPreviewContent(
  contentId: string,
  signal?: AbortSignal,
): Promise<PreviewContent> {
  const response = await fetch(`${API_BASE}/api/v1/contents/${contentId}/token-metadata`, {
    headers: { Accept: "application/json" },
    signal,
  });

  if (!response.ok) {
    let detail = "";

    try {
      const body = (await response.json()) as { detail?: string };
      detail = body.detail || "";
    } catch {
      detail = "";
    }

    throw new PreviewApiError(detail || fallbackMessage(response.status), response.status);
  }

  const metadata = (await response.json()) as TokenMetadataResponse;

  return tokenMetadataToPreviewContent(contentId, metadata);
}

export function getShareUrl(contentId: string): string {
  return `${API_BASE}/c/${contentId}`;
}

function tokenMetadataToPreviewContent(
  requestedContentId: string,
  metadata: TokenMetadataResponse,
): PreviewContent {
  const attributes = Array.isArray(metadata.attributes) ? metadata.attributes : [];
  const resolvedContentId = String(readAttribute(attributes, "Content ID") || requestedContentId).toLowerCase();
  const durationMs = parseDurationMs(
    readAttribute(attributes, "Duration") || metadata.description || "",
  );

  return {
    content_id: resolvedContentId,
    title: metadata.name || `SoundPola: 声音碎片 #${resolvedContentId.slice(0, 4).toUpperCase()}`,
    description: metadata.description || "",
    duration_ms: durationMs,
    created_at: String(readAttribute(attributes, "Created") || ""),
    audio_url: `${API_BASE}/preview/${resolvedContentId}/audio`,
    video_url: `${API_BASE}/preview/${resolvedContentId}/video`,
    on_chain: true,
    attributes,
    token_id: readAttribute(attributes, "Token ID") || null,
    contract_address: String(readAttribute(attributes, "Contract") || ""),
    editions_count: readNumberAttribute(attributes, "Editions"),
  };
}

function readAttribute(attributes: PreviewAttribute[], traitType: string): string | number | undefined {
  return attributes.find((attribute) => attribute.trait_type === traitType)?.value;
}

function readNumberAttribute(attributes: PreviewAttribute[], traitType: string): number | null {
  const value = readAttribute(attributes, traitType);
  const numberValue = typeof value === "number" ? value : Number(value);

  return Number.isFinite(numberValue) ? numberValue : null;
}

function parseDurationMs(input: string | number): number {
  if (typeof input === "number") return input;

  const match = input.match(/(\d+(?:\.\d+)?)\s*ms/i);
  if (!match) return 0;

  return Math.round(Number(match[1]));
}
