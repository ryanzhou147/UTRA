"use client";

import { useState, useCallback } from "react";
import DragDropZone from "@/components/DragDropZone";

interface Run {
  id: string;
  team: string;
  section: "target" | "obstacle";
  score: number;
  timestamp: string;
  videoUrl?: string;
  videoFile?: File;
  verified: boolean;
  hash?: string;
}

const initialRuns: Run[] = [
  { id: "run-001", team: "ROBO-042", section: "target", score: 2450, timestamp: "2024-01-31 14:32", videoUrl: "https://www.youtube.com/embed/dQw4w9WgXcQ", verified: true, hash: "a1b2c3..." },
  { id: "run-002", team: "MECH-017", section: "obstacle", score: 1920, timestamp: "2024-01-31 13:15", videoUrl: "https://www.youtube.com/embed/dQw4w9WgXcQ", verified: true, hash: "d4e5f6..." },
  { id: "run-003", team: "BOLT-099", section: "target", score: 1840, timestamp: "2024-01-31 11:42", verified: false },
  { id: "run-004", team: "TITAN-003", section: "obstacle", score: 2100, timestamp: "2024-01-31 10:30", verified: true, hash: "g7h8i9..." },
  { id: "run-005", team: "NOVA-021", section: "target", score: 1650, timestamp: "2024-01-31 09:15", verified: false },
  { id: "run-006", team: "ROBO-042", section: "obstacle", score: 2280, timestamp: "2024-01-30 16:45", verified: true, hash: "j1k2l3..." },
  { id: "run-007", team: "MECH-017", section: "target", score: 2010, timestamp: "2024-01-30 15:20", verified: true, hash: "m4n5o6..." },
  { id: "run-008", team: "BOLT-099", section: "obstacle", score: 1780, timestamp: "2024-01-30 14:00", verified: false },
];

export default function RunsPage() {
  const [runs, setRuns] = useState<Run[]>(initialRuns);
  const [selected, setSelected] = useState<Run | null>(null);
  const [showForm, setShowForm] = useState(false);
  const [section, setSection] = useState<"target" | "obstacle">("target");
  const [score, setScore] = useState("");
  const [team, setTeam] = useState("ROBO-042");
  const [videoFile, setVideoFile] = useState<File | null>(null);
  const [videoPreview, setVideoPreview] = useState<string | null>(null);

  const handleVideoFile = useCallback((file: File) => {
    setVideoFile(file);
    setVideoPreview(URL.createObjectURL(file));
  }, []);

  const handleAdd = () => {
    if (!score) return;
    const newRun: Run = {
      id: `run-${String(runs.length + 1).padStart(3, "0")}`,
      team,
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

  const getVideoSrc = (run: Run) => run.videoFile ? URL.createObjectURL(run.videoFile) : run.videoUrl;
  const isLocalVideo = (run: Run) => !!run.videoFile || (run.videoUrl?.startsWith("blob:"));

  return (
    <div className="max-w-7xl mx-auto">
      <div className="flex justify-between items-center mb-6">
        <div className="text-[#6c7086] text-sm">{">"} RUN HISTORY</div>
        <button onClick={() => setShowForm(!showForm)} className="text-[#cba6f7] text-sm hover:text-[#b4befe] transition-colors">
          [{showForm ? "CANCEL" : "+ ADD RUN"}]
        </button>
      </div>

      {showForm && (
        <div className="terminal-box p-5 mb-6 text-sm">
          <div className="text-[#cba6f7] mb-4 font-medium">NEW RUN</div>
          <div className="grid grid-cols-3 gap-4">
            <div>
              <label className="text-[#6c7086] text-xs block mb-2">TEAM</label>
              <select value={team} onChange={(e) => setTeam(e.target.value)} className="w-full bg-[#1e1e2e] border border-[#45475a] p-2 text-[#cdd6f4] rounded">
                <option>ROBO-042</option>
                <option>MECH-017</option>
                <option>BOLT-099</option>
                <option>TITAN-003</option>
                <option>NOVA-021</option>
              </select>
            </div>
            <div>
              <label className="text-[#6c7086] text-xs block mb-2">SECTION</label>
              <select value={section} onChange={(e) => setSection(e.target.value as "target" | "obstacle")} className="w-full bg-[#1e1e2e] border border-[#45475a] p-2 text-[#cdd6f4] rounded">
                <option value="target">target</option>
                <option value="obstacle">obstacle</option>
              </select>
            </div>
            <div>
              <label className="text-[#6c7086] text-xs block mb-2">SCORE</label>
              <input type="number" value={score} onChange={(e) => setScore(e.target.value)} className="w-full bg-[#1e1e2e] border border-[#45475a] p-2 text-[#cdd6f4] rounded" placeholder="0" />
            </div>
            <div className="col-span-3">
              <label className="text-[#6c7086] text-xs block mb-2">VIDEO</label>
              <DragDropZone accept={["video/*"]} onFile={handleVideoFile} label="DROP VIDEO" sublabel=".mp4, .mov, .webm" preview={videoPreview} className="h-32" />
            </div>
            <div className="col-span-3">
              <button onClick={handleAdd} className="bg-[#313145] border border-[#cba6f7] px-4 py-2 text-[#cba6f7] hover:bg-[#45475a] rounded">[SUBMIT]</button>
            </div>
          </div>
        </div>
      )}

      <div className="grid lg:grid-cols-5 gap-6">
        <div className="lg:col-span-3 terminal-box p-5 h-[70vh] overflow-y-auto">
          <table className="w-full text-sm">
            <thead>
              <tr className="text-[#6c7086] border-b border-[#45475a]">
                <th className="text-left py-3">ID</th>
                <th className="text-left py-3">TEAM</th>
                <th className="text-left py-3">TYPE</th>
                <th className="text-right py-3">SCORE</th>
                <th className="text-right py-3">STATUS</th>
              </tr>
            </thead>
            <tbody>
              {runs.map((run) => (
                <tr key={run.id} onClick={() => setSelected(run)} className={`border-b border-[#45475a]/50 cursor-pointer hover:bg-[#313145] transition-colors ${selected?.id === run.id ? "bg-[#313145]" : ""}`}>
                  <td className="py-3 text-[#cdd6f4]">{run.id}</td>
                  <td className="py-3 text-[#a6adc8]">{run.team}</td>
                  <td className="py-3 text-[#89b4fa]">{run.section}</td>
                  <td className="py-3 text-right text-[#cdd6f4] tabular-nums">{run.score}</td>
                  <td className="py-3 text-right">{run.verified ? <span className="text-[#a6e3a1]">[OK]</span> : <span className="text-[#fab387]">[PENDING]</span>}</td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>

        <div className="lg:col-span-2 terminal-box p-5 h-[70vh]">
          {selected ? (
            <div className="h-full flex flex-col">
              <div className="text-[#cba6f7] mb-3 font-medium">{selected.id.toUpperCase()}</div>
              <div className="text-sm text-[#a6adc8] mb-4 space-y-1">
                <div>Team: <span className="text-[#cdd6f4]">{selected.team}</span></div>
                <div>Section: <span className="text-[#89b4fa]">{selected.section}</span></div>
                <div>Score: <span className="text-[#cdd6f4]">{selected.score}</span></div>
                <div>Time: <span className="text-[#6c7086]">{selected.timestamp}</span></div>
                {selected.hash && <div>Hash: <span className="text-[#6c7086] font-mono">{selected.hash}</span></div>}
              </div>
              {selected.videoUrl || selected.videoFile ? (
                <div className="flex-1 bg-[#1e1e2e] border border-[#45475a] rounded min-h-0">
                  {isLocalVideo(selected) ? <video src={getVideoSrc(selected)} className="w-full h-full object-contain rounded" controls /> : <iframe src={selected.videoUrl} className="w-full h-full rounded" allowFullScreen />}
                </div>
              ) : (
                <div className="flex-1 bg-[#1e1e2e] border border-[#45475a] rounded flex items-center justify-center text-[#6c7086]">NO VIDEO</div>
              )}
            </div>
          ) : (
            <div className="h-full flex items-center justify-center text-[#6c7086]">SELECT A RUN</div>
          )}
        </div>
      </div>
    </div>
  );
}
