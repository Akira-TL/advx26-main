import { Button, Toast } from "antd-mobile";
import { Copy, ExternalLink, Pause, Play } from "lucide-react";
import type { MouseEvent } from "react";

import { ActionTooltip } from "./ActionTooltip";
import { getAppContentLink } from "../lib/deepLink";
import { formatClock, formatDate, formatDurationMs, shortAddress } from "../lib/format";
import { getShareUrl } from "../lib/previewApi";
import type { PreviewContent } from "../types/preview";
import { useLinkedPlayback } from "../hooks/useLinkedPlayback";

interface PlayerCardProps {
  content: PreviewContent;
}

export function PlayerCard({ content }: PlayerCardProps) {
  const playback = useLinkedPlayback(content.audio_url);
  const durationSeconds = formatDurationMs(content.duration_ms);
  const createdDate = formatDate(content.created_at);

  const handleSeek = (event: MouseEvent<HTMLButtonElement>) => {
    const rect = event.currentTarget.getBoundingClientRect();
    playback.seekToRatio((event.clientX - rect.left) / rect.width);
  };

  const copyLink = async () => {
    const link = getShareUrl(content.content_id);

    try {
      await navigator.clipboard.writeText(link);
      Toast.show({ content: "\u5df2\u590d\u5236\u58f0\u7247\u94fe\u63a5", position: "bottom" });
    } catch {
      window.prompt("\u8bf7\u624b\u52a8\u590d\u5236\u94fe\u63a5", link);
    }
  };

  const openApp = () => {
    window.location.href = getAppContentLink(content.content_id);
  };

  return (
    <section className="w-full max-w-[380px] overflow-hidden rounded-2xl border border-white/[0.06] bg-panel shadow-player">
      <button
        type="button"
        className="relative flex aspect-square w-full cursor-pointer items-center justify-center overflow-hidden bg-black text-left outline-none"
        onClick={playback.toggle}
        aria-label={playback.isPlaying ? "暂停播放" : "播放声片"}
      >
        <video
          ref={playback.videoRef}
          className="h-full w-full object-cover"
          loop
          muted
          playsInline
          preload="metadata"
          src={content.video_url}
        />
        <span
          className={[
            "absolute inline-flex h-12 w-12 items-center justify-center rounded-full bg-ink/45 text-textMain backdrop-blur-sm transition-opacity",
            playback.isPlaying ? "opacity-0" : "opacity-100",
          ].join(" ")}
        >
          {playback.isPlaying ? <Pause size={19} /> : <Play size={19} className="translate-x-0.5" />}
        </span>
      </button>

      <div className="px-6 pb-6 pt-5">
        <h1 className="mb-2 text-lg font-semibold leading-snug text-textMain">
          {content.title || "未命名声片"}
        </h1>
        {content.description ? (
          <p className="mb-3 text-sm leading-6 text-textMuted">{content.description}</p>
        ) : null}

        <div className="mb-4 flex flex-wrap items-center gap-2 text-[13px] text-textMuted">
          <span
            className={[
              "inline-flex items-center gap-1 rounded-full border px-2.5 py-1 text-[11px] font-medium",
              content.on_chain
                ? "border-gold/30 bg-gold/10 text-gold"
                : "border-textMuted/25 bg-textMuted/10 text-textMuted",
            ].join(" ")}
          >
            {content.on_chain ? "◆ 已上链" : "◇ 仅预览"}
          </span>
          <span>{durationSeconds}s</span>
          <span>{createdDate}</span>
        </div>

        <ActionTooltip label="点击进度条跳转">
          <button
            type="button"
            className="block h-3 w-full rounded-full py-1 outline-none"
            onClick={handleSeek}
            aria-label="播放进度"
          >
            <span className="block h-[3px] overflow-hidden rounded-full bg-white/[0.08]">
              <span
                className="block h-full rounded-full bg-mint transition-[width] duration-100"
                style={{ width: `${playback.progress * 100}%` }}
              />
            </span>
          </button>
        </ActionTooltip>

        <div className="mt-1 flex justify-between text-[11px] text-textFaint">
          <span>{formatClock(playback.currentTime)}</span>
          <span>{durationSeconds}s</span>
        </div>

        {playback.error ? <p className="mt-3 text-xs text-coral">{playback.error}</p> : null}

        <div className="mt-4 border-t border-white/[0.06] pt-4">
          <DetailRow label="内容 ID" value={`${content.content_id.slice(0, 12)}…`} mono />
          <DetailRow label="时长" value={`${durationSeconds} 秒`} />
          <DetailRow label="创建" value={createdDate} />
        </div>

        <div className="mt-4 rounded-xl bg-panelSoft p-3.5">
          {content.on_chain ? (
            <>
              {content.token_id ? <ChainRow label="Token" value={`#${content.token_id}`} /> : null}
              {content.contract_address ? (
                <ChainRow label="合约" value={shortAddress(content.contract_address)} mono />
              ) : null}
              {typeof content.editions_count === "number" ? (
                <ChainRow label="收藏" value={`${content.editions_count} 人已领取`} />
              ) : null}
              {!content.token_id && !content.contract_address && typeof content.editions_count !== "number" ? (
                <p className="py-1 text-center text-xs text-textFaint">链上元数据已生成</p>
              ) : null}
            </>
          ) : (
            <p className="py-1 text-center text-xs text-textFaint">此声音尚未上链，仅供试听预览</p>
          )}
        </div>

        <div className="mt-4 grid gap-2.5">
          {content.on_chain ? (
            <Button
              block
              color="primary"
              className="soundpola-primary-button"
              onClick={openApp}
            >
              <span className="inline-flex items-center gap-2">
                <ExternalLink size={16} />
                在 SoundPola 中领取
              </span>
            </Button>
          ) : null}

          <Button block className="soundpola-secondary-button" onClick={copyLink}>
            <span className="inline-flex items-center gap-2">
              <Copy size={16} />
              复制声片链接
            </span>
          </Button>
        </div>

        {content.on_chain ? (
          <p className="mt-2 text-center text-[11px] text-textFaint">
            打开 App 将此声音添加到你的链上钱包
          </p>
        ) : null}
      </div>
    </section>
  );
}

interface DetailRowProps {
  label: string;
  value: string;
  mono?: boolean;
}

function DetailRow({ label, value, mono = false }: DetailRowProps) {
  return (
    <div className="mb-1.5 flex items-center justify-between gap-4 text-xs text-textMuted last:mb-0">
      <span>{label}</span>
      <span className={["font-medium text-textMain", mono ? "font-mono text-[11px]" : ""].join(" ")}>
        {value}
      </span>
    </div>
  );
}

function ChainRow({ label, value, mono = false }: DetailRowProps) {
  return (
    <div className="mb-1.5 flex items-center justify-between gap-4 text-xs last:mb-0">
      <span className="text-textFaint">{label}</span>
      <span className={["font-medium text-textMain", mono ? "font-mono text-[11px]" : ""].join(" ")}>
        {value}
      </span>
    </div>
  );
}
