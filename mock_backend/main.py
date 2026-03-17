"""
Mock FastAPI backend for Interactive Pitch Analysis Viewer.

Endpoints:
  GET  /health
  POST /analyze/upload
  GET  /database/pitchers
  GET  /database/pitcher/{pitcher_id}

Run:
  uvicorn main:app --port 8001 --reload
"""

import asyncio
from fastapi import FastAPI, UploadFile, File, HTTPException
from fastapi.middleware.cors import CORSMiddleware
import mock_data

app = FastAPI(title="Pitch Analysis API (Mock)", version="1.0.0-mock")

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)


@app.get("/health")
async def health():
    return {"status": "ok", "version": "1.0.0-mock"}


@app.post("/analyze/upload")
async def analyze_upload(file: UploadFile = File(...)):
    """
    Accept a video file upload and return mock analysis results.
    The file content is ignored — realistic mock data is returned.
    A short delay simulates processing time so the progress bar animates.
    """
    # Drain the upload so progress signals fire in the C++ client
    await file.read()

    # Simulate analysis processing time (1.5 s)
    await asyncio.sleep(1.5)

    return mock_data.ANALYSIS_RESPONSE


@app.get("/database/pitchers")
async def get_pitchers():
    return mock_data.PITCHER_LIST


@app.get("/database/pitcher/{pitcher_id}")
async def get_pitcher(pitcher_id: str):
    pitcher = mock_data.PITCHER_MAP.get(pitcher_id)
    if pitcher is None:
        raise HTTPException(status_code=404, detail=f"Pitcher '{pitcher_id}' not found")
    return pitcher
