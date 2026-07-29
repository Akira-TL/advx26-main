export interface PreviewContent {
  content_id: string;
  title: string;
  description: string;
  duration_ms: number;
  created_at: string;
  audio_url: string;
  video_url: string;
  on_chain: boolean;
  attributes: PreviewAttribute[];
  token_id?: string | number | null;
  tx_hash?: string | null;
  contract_address?: string | null;
  editions_count?: number | null;
}

export interface PreviewAttribute {
  trait_type: string;
  value: string | number;
}

export type LoadState =
  | { status: "idle" }
  | { status: "loading" }
  | { status: "error"; message: string }
  | { status: "ready"; content: PreviewContent };
