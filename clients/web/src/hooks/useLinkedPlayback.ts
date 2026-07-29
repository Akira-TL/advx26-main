import { Howl } from "howler";
import { useCallback, useEffect, useMemo, useRef, useState } from "react";

interface LinkedPlayback {
  currentTime: number;
  duration: number;
  error: string;
  isPlaying: boolean;
  progress: number;
  pause: () => void;
  seekToRatio: (ratio: number) => void;
  toggle: () => void;
  videoRef: React.RefObject<HTMLVideoElement>;
}

export function useLinkedPlayback(audioUrl: string): LinkedPlayback {
  const howlRef = useRef<Howl | null>(null);
  const rafRef = useRef<number>();
  const videoRef = useRef<HTMLVideoElement>(null);
  const [currentTime, setCurrentTime] = useState(0);
  const [duration, setDuration] = useState(0);
  const [isPlaying, setIsPlaying] = useState(false);
  const [error, setError] = useState("");

  const clearFrame = useCallback(() => {
    if (rafRef.current) {
      window.cancelAnimationFrame(rafRef.current);
      rafRef.current = undefined;
    }
  }, []);

  const readTime = useCallback(() => {
    const howl = howlRef.current;
    const seek = howl?.seek();
    const nextTime = typeof seek === "number" ? seek : 0;

    setCurrentTime(nextTime);
    rafRef.current = window.requestAnimationFrame(readTime);
  }, []);

  const pause = useCallback(() => {
    howlRef.current?.pause();
    videoRef.current?.pause();
    setIsPlaying(false);
    clearFrame();
  }, [clearFrame]);

  const play = useCallback(() => {
    const howl = howlRef.current;

    if (!howl) return;

    setError("");
    howl.play();

    const video = videoRef.current;
    if (video) {
      const seek = howl.seek();
      if (typeof seek === "number" && Number.isFinite(seek)) {
        video.currentTime = seek;
      }
      void video.play().catch(() => undefined);
    }

    setIsPlaying(true);
  }, []);

  const toggle = useCallback(() => {
    if (isPlaying) {
      pause();
    } else {
      play();
    }
  }, [isPlaying, pause, play]);

  const seekToRatio = useCallback(
    (ratio: number) => {
      const howl = howlRef.current;
      const clamped = Math.max(0, Math.min(1, ratio));
      const nextTime = clamped * duration;

      if (!howl || !duration) return;

      howl.seek(nextTime);
      setCurrentTime(nextTime);

      const video = videoRef.current;
      if (video && Number.isFinite(video.duration)) {
        video.currentTime = Math.min(nextTime, video.duration);
      }
    },
    [duration],
  );

  useEffect(() => {
    clearFrame();
    setCurrentTime(0);
    setDuration(0);
    setIsPlaying(false);
    setError("");

    if (!audioUrl) return undefined;

    const howl = new Howl({
      src: [audioUrl],
      html5: true,
      preload: true,
      onload: () => setDuration(howl.duration()),
      onplay: () => {
        setDuration(howl.duration());
        setIsPlaying(true);
        clearFrame();
        rafRef.current = window.requestAnimationFrame(readTime);
      },
      onpause: () => {
        setIsPlaying(false);
        clearFrame();
      },
      onend: () => {
        setIsPlaying(false);
        setCurrentTime(0);
        clearFrame();
        howl.seek(0);
        if (videoRef.current) {
          videoRef.current.pause();
          videoRef.current.currentTime = 0;
        }
      },
      onloaderror: () => setError("音频加载失败，请稍后重试"),
      onplayerror: () => setError("浏览器阻止了播放，请再次点击画面"),
    });

    howlRef.current = howl;

    return () => {
      clearFrame();
      howl.unload();
      howlRef.current = null;
    };
  }, [audioUrl, clearFrame, readTime]);

  const progress = useMemo(() => {
    if (!duration) return 0;
    return Math.max(0, Math.min(1, currentTime / duration));
  }, [currentTime, duration]);

  return {
    currentTime,
    duration,
    error,
    isPlaying,
    progress,
    pause,
    seekToRatio,
    toggle,
    videoRef,
  };
}
