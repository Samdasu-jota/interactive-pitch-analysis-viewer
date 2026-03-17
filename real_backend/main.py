"""
Real FastAPI backend — Interactive Pitch Analysis Viewer.
Uses MediaPipe Pose for actual biomechanical analysis.

Run:
  source venv/bin/activate
  uvicorn main:app --port 8001 --reload
"""

import sys, os
sys.path.insert(0, os.path.dirname(__file__))

from fastapi import FastAPI, UploadFile, File, HTTPException
from fastapi.middleware.cors import CORSMiddleware

import analyzer

# Reuse pitcher DB and health response from mock_data
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'mock_backend'))
from mock_data import PITCHER_LIST, PITCHER_MAP

app = FastAPI(title="Pitch Analysis API (MediaPipe)", version="2.0.0")

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)


@app.get("/health")
async def health():
    return {"status": "ok", "version": "2.0.0-mediapipe"}


@app.post("/analyze/upload")
async def analyze_upload(file: UploadFile = File(...)):
    """
    Upload a pitch video. MediaPipe Pose runs on every frame.
    Returns pose landmarks, 14 biomechanical metrics, pitch phases,
    and top-3 pitcher similarity matches.
    """
    data = await file.read()
    if not data:
        raise HTTPException(status_code=400, detail="Empty file upload.")

    try:
        result = analyzer.analyze_video_bytes(data)
    except RuntimeError as e:
        raise HTTPException(status_code=422, detail=str(e))

    return result


@app.get("/database/pitchers")
async def get_pitchers():
    return PITCHER_LIST


@app.get("/database/pitcher/{pitcher_id}")
async def get_pitcher(pitcher_id: str):
    pitcher = PITCHER_MAP.get(pitcher_id)
    if pitcher is None:
        raise HTTPException(status_code=404, detail=f"Pitcher '{pitcher_id}' not found")
    return pitcher
