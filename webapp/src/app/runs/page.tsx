"use client";

import { useState, useCallback } from "react";
import DragDropZone from "@/components/DragDropZone";

interface Run {
  id: string;
  section: "target" | "obstacle";
  score: number;
  timestamp: string;
  videoUrl?: string;
  videoFile?: File;
  verified: boolean;
}

const initialRuns: Run[] = [
  { id: "run-001", section: "target", score: 2450, timestamp: "2024-01-31 14:32", videoUrl: "https://www.youtube.com/embed/dQw4w9WgXcQ", verified: true },
  { id: "run-002", section: "obstacle", score: 1920, timestamp: "2024-01-31 13:15", videoUrl: "https://www.youtube.com/embed/dQw4w9WgXcQ", verified: true },
  { id: "run-003", section: "target", score: 1840, timestamp: "2024-01-31 11:42", verified: false },
];

export default function RunsPage() {
  const [runs, setRuns] = useState<Run[]>(initialRuns);
  const [selected, setSelected] = useState<Run | null>(null);
  const [showForm, setShowForm] = useState(false);

  // Form state
  const [section, setSection] = useState<"target" | "obstacle">("target");
  const [score, setScore] = useState("");
  const [videoFile, setVideoFile] = useState<File | null>(null);
  const [videoPreview, setVideoPreview] = useState<string | null>(null);

  const handleVideoFile = useCallback((file: File) => {
    setVideoFile(file);
    const url = URL.createObjectURL(file);
    setVideoPreview(url);
  }, []);

  const handleAdd = () => {
    if (!score) return;
    const newRun: Run = {
      id: `run-${String(runs.length + 1).padStart(3, "0")}`,
      section,
      score: parseInt(score),
      timestamp: new Date().toISOString().slice(0, 16).replace("T", " "),
      videoUrl: videoPreview || undefined,
      videoFile: videoFile || undefined,
      verified: false,
    };
    setRuns([newRun, ...runs]);
    setShowForm(false);
    setScore("");
    setVideoFile(null);
    setVideoPreview(null);
  };

  const getVideoSrc = (run: Run) => {
    if (run.videoFile) {
      return URL.createObjectURL(run.videoFile);
    }
    return run.videoUrl;
  };

  const isLocalVideo = (run: Run) => {
    return !!run.videoFile || (run.videoUrl && run.videoUrl.startsWith("blob:"));
  };

  return (
    <div className="max-w-5xl mx-auto">
      <div className="flex justify-between items-center mb-6">
        <div className="text-[#6c7086] text-sm">{">"} RUN HISTORY</div>
        <button
          onClick={() => setShowForm(!showForm)}
          className="text-[#cba6f7] text-sm hover:text-[#b4befe] transition-colors"
        >
          [{showForm ? "CANCEL" : "+ ADD RUN"}]
        </button>
      </div>

      {/* Add Form */}
      {showForm && (
        <div className="terminal-box p-5 mb-4 text-sm">
          <div className="text-[#cba6f7] mb-4 font-medium">NEW RUN</div>
          <div className="grid grid-cols-2 gap-4">
            <div>
              <label className="text-[#6c7086] text-xs block mb-2">SECTION</label>
              <select
                value={section}
                onChange={(e) => setSection(e.target.value as "target" | "obstacle")}
                className="w-full bg-[#1e1e2e] border border-[#45475a] p-2 text-[#cdd6f4] rounded focus:border-[#cba6f7] outline-none transition-colors"
              >
                <option value="target">target</option>
                <option value="obstacle">obstacle</option>
              </select>
            </div>
            <div>
              <label className="text-[#6c7086] text-xs block mb-2">SCORE</label>
              <input
                type="number"
                value={score}
                onChange={(e) => setScore(e.target.value)}
                className="w-full bg-[#1e1e2e] border border-[#45475a] p-2 text-[#cdd6f4] rounded focus:border-[#cba6f7] outline-none transition-colors"
                placeholder="0"
              />
            </div>
            <div className="col-span-2">
              <label className="text-[#6c7086] text-xs block mb-2">VIDEO FILE</label>
              <DragDropZone
                accept={["video/*"]}
                onFile={handleVideoFile}
                label="DROP VIDEO FILE"
                sublabel="or click to browse (.mp4, .mov, .webm)"
                preview={videoPreview}
                className="h-40"
              />
            </div>
            <div className="col-span-2">
              <button
                onClick={handleAdd}
                className="bg-[#313145] border border-[#cba6f7] px-4 py-2 text-[#cba6f7] hover:bg-[#45475a] rounded transition-colors"
              >
                [SUBMIT]
              </button>
            </div>
          </div>
        </div>
      )}

      <div className="grid md:grid-cols-2 gap-4">
        {/* Run List */}
        <div className="terminal-box p-5 h-[60vh] overflow-y-auto">
          <table className="w-full text-sm">
            <thead>
              <tr className="text-[#6c7086] border-b border-[#45475a]">
                <th className="text-left py-3">ID</th>
                <th className="text-left py-3">TYPE</th>
                <th className="text-right py-3">SCORE</th>
                <th className="text-right py-3">STATUS</th>
              </tr>
            </thead>
            <tbody>
              {runs.map((run) => (
                <tr
                  key={run.id}
                  onClick={() => setSelected(run)}
                  className={`border-b border-[#45475a]/50 cursor-pointer hover:bg-[#313145] transition-colors ${
                    selected?.id === run.id ? "bg-[#313145]" : ""
                  }`}
                >
                  <td className="py-3 text-[#cdd6f4]">{run.id}</td>
                  <td className="py-3 text-[#89b4fa]">{run.section}</td>
                  <td className="py-3 text-right text-[#cdd6f4] tabular-nums">{run.score}</td>
                  <td className="py-3 text-right">
                    {run.verified ? (
                      <span className="text-[#a6e3a1]">[OK]</span>
                    ) : (
                      <span className="text-[#fab387]">[PENDING]</span>
                    )}
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>

        {/* Video Player */}
        <div className="terminal-box p-5 h-[60vh]">
          {selected ? (
            <div className="h-full flex flex-col">
              <div className="text-[#cba6f7] mb-3 font-medium">{selected.id.toUpperCase()}</div>
              <div className="text-sm text-[#a6adc8] mb-4 space-y-1">
                <div>Section: <span className="text-[#89b4fa]">{selected.section}</span></div>
                <div>Score: <span className="text-[#cdd6f4]">{selected.score}</span></div>
                <div>Time: <span className="text-[#6c7086]">{selected.timestamp}</span></div>
              </div>
              {selected.videoUrl || selected.videoFile ? (
                <div className="flex-1 bg-[#1e1e2e] border border-[#45475a] rounded min-h-0">
                  {isLocalVideo(selected) ? (
                    <video
                      src={getVideoSrc(selected)}
                      className="w-full h-full object-contain rounded"
                      controls
                    />
                  ) : (
                    <iframe
                      src={selected.videoUrl}
                      className="w-full h-full rounded"
                      allowFullScreen
                    />
                  )}
                </div>
              ) : (
                <div className="flex-1 bg-[#1e1e2e] border border-[#45475a] rounded flex items-center justify-center text-[#6c7086]">
                  NO VIDEO
                </div>
              )}
            </div>
          ) : (
            <div className="h-full flex items-center justify-center text-[#6c7086]">
              SELECT A RUN
            </div>
          )}
        </div>
      </div>
    </div>
  );
}
