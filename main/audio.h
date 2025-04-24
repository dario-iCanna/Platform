#pragma once

#include <xaudio2.h>

#define audio IXAudio2SourceVoice*
#define audioBuffer XAUDIO2_BUFFER

#ifndef _XBOX
#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'
#endif

void InizializzaAudio();

HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition);

HRESULT ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset);

IXAudio2SourceVoice* LeggiSuono(const TCHAR* filename, XAUDIO2_BUFFER& buffer);

audio PlayAudio(const TCHAR *filename, XAUDIO2_BUFFER& buffer, int loopCount, float volume);

void StopAudio(audio suono);