import { DotLoading } from "antd-mobile";
import { FormEvent, useEffect, useMemo, useState } from "react";

import { BrandMark } from "../components/BrandMark";
import { PlayerCard } from "../components/PlayerCard";
import { extractContentId, getInitialContentId } from "../lib/contentId";
import { fetchPreviewContent } from "../lib/previewApi";
import type { LoadState } from "../types/preview";

export function PreviewPage() {
  const [inputValue, setInputValue] = useState("");
  const [inputError, setInputError] = useState("");
  const [loadState, setLoadState] = useState<LoadState>(() => {
    return getInitialContentId() ? { status: "loading" } : { status: "idle" };
  });

  const initialContentId = useMemo(() => getInitialContentId(), []);

  const load = async (contentId: string) => {
    const id = extractContentId(contentId);
    if (!id) {
      setInputError("无法识别，请检查链接或 ID 格式");
      return;
    }

    const controller = new AbortController();
    setLoadState({ status: "loading" });
    setInputError("");

    try {
      const content = await fetchPreviewContent(id, controller.signal);
      setLoadState({ status: "ready", content });
    } catch (error) {
      const message =
        error instanceof Error && error.message === "Failed to fetch"
          ? "网络异常，无法连接服务器"
          : error instanceof Error
            ? error.message
            : "加载失败";

      setLoadState({ status: "error", message });
      setTimeout(() => {
        setLoadState({ status: "idle" });
        setInputError(message);
      }, 1600);
    }
  };

  useEffect(() => {
    if (initialContentId) {
      void load(initialContentId);
    }
  }, [initialContentId]);

  const submit = (event: FormEvent<HTMLFormElement>) => {
    event.preventDefault();
    void load(inputValue);
  };

  return (
    <main className="flex min-h-dvh flex-col items-center justify-center bg-ink px-6 py-8 text-textMain">
      {loadState.status === "idle" ? (
        <form className="w-full max-w-[380px] text-center" onSubmit={submit}>
          <BrandMark size="lg" />
          <h1 className="mt-4 text-xl font-bold">SoundPola 声片预览</h1>
          <p className="mb-6 mt-2 text-sm text-textMuted">粘贴声片链接，或输入 32 位内容 ID</p>
          <input
            className="mb-3 w-full rounded-xl border border-white/10 bg-panel px-4 py-3.5 text-sm text-textMain outline-none transition placeholder:text-textFaint focus:border-mint/50"
            placeholder="http://…/c/xxxx 或 32 位 ID"
            autoComplete="off"
            value={inputValue}
            onChange={(event) => setInputValue(event.target.value)}
          />
          <button
            type="submit"
            className="w-full rounded-xl bg-gradient-to-br from-mint to-jade px-4 py-3.5 text-sm font-semibold text-ink transition active:opacity-85"
          >
            打开声片
          </button>
          <p className="mt-3 min-h-5 text-xs text-coral">{inputError}</p>
        </form>
      ) : null}

      {loadState.status === "loading" ? (
        <p className="inline-flex items-center gap-2 text-sm text-textMuted">
          <DotLoading color="currentColor" />
          加载中...
        </p>
      ) : null}

      {loadState.status === "error" ? <p className="text-center text-sm text-textMuted">{loadState.message}</p> : null}

      {loadState.status === "ready" ? (
        <>
          <PlayerCard content={loadState.content} />
          <footer className="mt-6 text-center text-xs text-textFaint">
            由 <a href="#" className="text-mint no-underline">SoundPola</a> 铸造 · 声音记忆上链存证
          </footer>
        </>
      ) : null}
    </main>
  );
}
