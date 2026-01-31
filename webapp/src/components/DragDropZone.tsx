"use client";

import { useState, useCallback, DragEvent } from "react";

interface DragDropZoneProps {
  accept: string[];
  onFile: (file: File) => void;
  label: string;
  sublabel?: string;
  preview?: string | null;
  className?: string;
}

export default function DragDropZone({
  accept,
  onFile,
  label,
  sublabel,
  preview,
  className = "",
}: DragDropZoneProps) {
  const [isDragging, setIsDragging] = useState(false);

  const handleDrag = useCallback((e: DragEvent) => {
    e.preventDefault();
    e.stopPropagation();
  }, []);

  const handleDragIn = useCallback((e: DragEvent) => {
    e.preventDefault();
    e.stopPropagation();
    setIsDragging(true);
  }, []);

  const handleDragOut = useCallback((e: DragEvent) => {
    e.preventDefault();
    e.stopPropagation();
    setIsDragging(false);
  }, []);

  const handleDrop = useCallback(
    (e: DragEvent) => {
      e.preventDefault();
      e.stopPropagation();
      setIsDragging(false);

      const files = e.dataTransfer.files;
      if (files && files.length > 0) {
        const file = files[0];
        const fileType = file.type;
        const isAccepted = accept.some(
          (type) =>
            fileType === type ||
            (type.endsWith("/*") && fileType.startsWith(type.replace("/*", "/")))
        );
        if (isAccepted) {
          onFile(file);
        }
      }
    },
    [accept, onFile]
  );

  const handleClick = () => {
    const input = document.createElement("input");
    input.type = "file";
    input.accept = accept.join(",");
    input.onchange = (e) => {
      const file = (e.target as HTMLInputElement).files?.[0];
      if (file) onFile(file);
    };
    input.click();
  };

  return (
    <div
      onClick={handleClick}
      onDragEnter={handleDragIn}
      onDragLeave={handleDragOut}
      onDragOver={handleDrag}
      onDrop={handleDrop}
      className={`
        border-2 border-dashed cursor-pointer transition-all rounded-lg
        ${isDragging ? "border-[#cba6f7] bg-[#cba6f7]/10" : "border-[#45475a] hover:border-[#6c7086]"}
        ${className}
      `}
    >
      {preview ? (
        <div className="relative w-full h-full">
          {accept.some((t) => t.startsWith("video/")) ? (
            <video src={preview} className="w-full h-full object-contain rounded-lg" controls />
          ) : (
            <img src={preview} alt="Preview" className="w-full h-full object-contain rounded-lg" />
          )}
          <div className="absolute bottom-2 right-2 text-xs text-[#6c7086] bg-[#1e1e2e]/80 px-2 py-1 rounded">
            [DROP TO REPLACE]
          </div>
        </div>
      ) : (
        <div className="flex flex-col items-center justify-center h-full p-4 text-center">
          <div className="text-[#6c7086] mb-2">
            <pre className="text-2xl">
{`  ┌───┐
  │ ↓ │
  └───┘`}
            </pre>
          </div>
          <div className="text-[#a6adc8] text-sm">{label}</div>
          {sublabel && <div className="text-[#6c7086] text-xs mt-1">{sublabel}</div>}
        </div>
      )}
    </div>
  );
}
