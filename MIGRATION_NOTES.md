# TrickSaber Quest Migration Notes

## Critical Compatibility Issues Identified

### Beat Saber Version Gap
- **Current Target**: v1.37.0_9064817954 (OUTDATED)
- **Current Ecosystem**: v1.40.x series (1.40.8_7379 latest)
- **Impact**: Major version gap requiring significant refactoring

### Dependency Version Mismatches
- **bs-cordl**: Using v3700.0.0 (likely incompatible with 1.40.x)
- **BSML**: Using v0.4.43 (current is v0.4.53+)
- **custom-types**: Using v0.17.10 (current is v0.18.3+)

## Questions for Internet-Connected LLM

### 1. Beat Saber API Changes (1.37 → 1.40.x)
- Hook signature changes for core methods:
  - `SaberManager::Start`
  - `AudioTimeSyncController::Update`
  - `BeatmapObjectSpawnController::HandleNoteDataCallback`
- GlobalNamespace class structure changes
- IL2CPP metadata compatibility

### 2. Current Dependency Matrix
- Exact bs-cordl version for Beat Saber 1.40.8_7379
- Compatible beatsaber-hook version
- Scotland2 compatibility with 1.40.x
- BSML breaking changes from 0.4.43 → 0.4.53

### 3. Build System Updates
- NDK r25+ → r27 migration requirements
- CMake configuration changes
- ModsBeforeFriday vs QuestPatcher differences

### 4. Memory Management Changes
- IL2CPP runtime changes affecting object pooling
- GC behavior differences in newer Unity versions
- Circuit breaker pattern effectiveness

### 5. Hook Installation Patterns
- Scotland2 v0.1.6 load order guarantees
- Modular hook system compatibility
- Error handling best practices for 1.40.x

## Migration Strategy (When Ready)

### Phase 1: Environment Setup
1. Update to Beat Saber 1.40.x development environment
2. Install ModsBeforeFriday
3. Update NDK to r27

### Phase 2: Dependency Updates
1. Find correct bs-cordl version for target Beat Saber build
2. Update all core dependencies to compatible versions
3. Test basic mod loading

### Phase 3: API Migration
1. Update hook signatures
2. Fix GlobalNamespace references
3. Test core functionality

### Phase 4: Feature Restoration
1. Restore trick system functionality
2. Update UI components
3. Performance optimization

## Current Status
- Project functional on Beat Saber 1.37
- Major migration required for current ecosystem
- Recommend staying on 1.37 until migration resources available