# GHOSTYPE 서버 API 아키텍처

## 개요
GHOSTYPE 백엔드 서버는 **현재 미구현 상태**입니다. 이 문서는 향후 구현 예정인 서버 아키텍처의 설계 명세서입니다. 

**⚠️ 현재 상태**: 클라이언트(웹/iOS)가 ESP32와 직접 BLE 통신하는 구조로 동작중

> 📖 **전체 시스템 아키텍처**: [CLAUDE.md](../CLAUDE.md) 참조

## 핵심 역할
- **AI 서비스 통합**: ChatGPT, Claude API와 안전한 연동
- **텍스트 전처리**: 한글/영문 분석 및 QWERTY 키 시퀀스 변환
- **OCR 처리**: Google Vision API를 통한 화면 텍스트 추출
- **명령 생성**: ESP32용 프로토콜 명령 시퀀스 생성
- **사용자 관리**: 인증, 권한, 사용 이력 관리

## 기술 스택

### 서버 프레임워크
- **Node.js + Express**: RESTful API 서버
- **Python FastAPI**: AI/ML 처리 마이크로서비스
- **Redis**: 작업 큐 및 캐싱
- **PostgreSQL**: 사용자 데이터 및 히스토리

### 외부 서비스
- **OpenAI API**: GPT-4 텍스트 생성
- **Anthropic API**: Claude 텍스트 생성
- **Google Vision API**: OCR 텍스트 추출

## API 엔드포인트 설계

### 인증 API (`/api/auth`)
```typescript
POST /api/auth/register     // 회원가입
POST /api/auth/login        // 로그인
POST /api/auth/refresh      // 토큰 갱신
POST /api/auth/logout       // 로그아웃
PUT  /api/auth/password     // 비밀번호 변경
```

### AI 서비스 API (`/api/ai`)
```typescript
POST /api/ai/chat          // AI 채팅 요청
{
  "prompt": string,        // 사용자 프롬프트
  "model": "gpt-4" | "claude",  // AI 모델 선택
  "context": Message[]     // 이전 대화 컨텍스트
}

// 응답
{
  "response": string,      // AI 생성 텍스트
  "tokens": number,        // 사용 토큰 수
  "model": string          // 사용된 모델
}
```

### 텍스트 전처리 API (`/api/preprocess`)
```typescript
POST /api/preprocess/text   // 텍스트 → ESP32 명령
{
  "text": string,          // 원본 텍스트
  "options": {
    "speed": number,       // 타이핑 속도 (ms)
    "initialMode": "en" | "ko"  // 초기 언어
  }
}

// 응답
{
  "commands": [
    { "type": "INIT_ENGLISH" },
    { "type": "TYPE_TEXT", "content": "Hello" },
    { "type": "TOGGLE_LANGUAGE" },
    { "type": "TYPE_TEXT", "content": "dkssudgkseptpdy" }
  ],
  "totalLength": number,
  "estimatedTime": number   // 예상 소요 시간 (ms)
}
```

### OCR API (`/api/ocr`)
```typescript
POST /api/ocr/analyze      // 이미지 분석
{
  "image": string,         // Base64 인코딩 이미지
  "analyzeError": boolean  // AI 오류 분석 요청
}

// 응답
{
  "text": string,          // 추출된 텍스트
  "confidence": number,    // OCR 신뢰도
  "analysis": {            // AI 분석 (선택적)
    "errorType": string,
    "solution": string,
    "correctedCode": string
  }
}
```

### 히스토리 API (`/api/history`)
```typescript
GET  /api/history          // 전송 이력 조회
POST /api/history          // 새 이력 저장
GET  /api/history/:id      // 특정 이력 상세
DELETE /api/history/:id    // 이력 삭제

// 이력 구조
{
  "id": string,
  "title": string,         // AI 생성 제목
  "content": string,       // 원본 텍스트
  "commands": Command[],   // 생성된 명령
  "timestamp": Date,
  "deviceId": string       // ESP32 장치 ID
}
```

## 텍스트 전처리 엔진

### 언어 분석 알고리즘
```javascript
class TextPreprocessor {
  // 텍스트를 언어별 블록으로 분리
  analyzeLanguageBlocks(text) {
    const blocks = [];
    let currentBlock = { language: null, content: '' };
    
    for (const char of text) {
      const charLang = this.detectCharLanguage(char);
      
      if (charLang !== currentBlock.language) {
        if (currentBlock.content) {
          blocks.push({...currentBlock});
        }
        currentBlock = { language: charLang, content: char };
      } else {
        currentBlock.content += char;
      }
    }
    
    if (currentBlock.content) {
      blocks.push(currentBlock);
    }
    
    return blocks;
  }
  
  // 문자 언어 감지
  detectCharLanguage(char) {
    const code = char.charCodeAt(0);
    
    // 한글 범위: 0xAC00 ~ 0xD7A3
    if (code >= 0xAC00 && code <= 0xD7A3) {
      return 'korean';
    }
    
    // 영문 및 기타
    return 'english';
  }
}
```

### 한글 QWERTY 변환
```javascript
// 한글 자모 → QWERTY 매핑
const HANGUL_TO_QWERTY = {
  // 초성
  'ㄱ': 'r', 'ㄲ': 'R', 'ㄴ': 's', 'ㄷ': 'e', 'ㄸ': 'E',
  'ㄹ': 'f', 'ㅁ': 'a', 'ㅂ': 'q', 'ㅃ': 'Q', 'ㅅ': 't',
  'ㅆ': 'T', 'ㅇ': 'd', 'ㅈ': 'w', 'ㅉ': 'W', 'ㅊ': 'c',
  'ㅋ': 'z', 'ㅌ': 'x', 'ㅍ': 'v', 'ㅎ': 'g',
  
  // 중성
  'ㅏ': 'k', 'ㅐ': 'o', 'ㅑ': 'i', 'ㅒ': 'O', 'ㅓ': 'j',
  'ㅔ': 'p', 'ㅕ': 'u', 'ㅖ': 'P', 'ㅗ': 'h', 'ㅘ': 'hk',
  'ㅙ': 'ho', 'ㅚ': 'hl', 'ㅛ': 'y', 'ㅜ': 'n', 'ㅝ': 'nj',
  'ㅞ': 'np', 'ㅟ': 'nl', 'ㅠ': 'b', 'ㅡ': 'm', 'ㅢ': 'ml',
  'ㅣ': 'l'
  
  // 종성은 초성과 동일
};

// 한글 음절 분해
function decomposeHangul(syllable) {
  const code = syllable.charCodeAt(0) - 0xAC00;
  const final = code % 28;
  const medial = ((code - final) / 28) % 21;
  const initial = ((code - final) / 28 - medial) / 21;
  
  return {
    initial: INITIAL_CONSONANTS[initial],
    medial: MEDIAL_VOWELS[medial],
    final: final > 0 ? FINAL_CONSONANTS[final - 1] : null
  };
}
```

## 보안 및 인증

### JWT 기반 인증
```javascript
// 토큰 생성
function generateTokens(userId) {
  const accessToken = jwt.sign(
    { userId, type: 'access' },
    process.env.JWT_SECRET,
    { expiresIn: '15m' }
  );
  
  const refreshToken = jwt.sign(
    { userId, type: 'refresh' },
    process.env.JWT_REFRESH_SECRET,
    { expiresIn: '7d' }
  );
  
  return { accessToken, refreshToken };
}

// 미들웨어
function authenticateToken(req, res, next) {
  const token = req.headers['authorization']?.split(' ')[1];
  
  if (!token) {
    return res.status(401).json({ error: 'Token required' });
  }
  
  jwt.verify(token, process.env.JWT_SECRET, (err, user) => {
    if (err) return res.status(403).json({ error: 'Invalid token' });
    req.user = user;
    next();
  });
}
```

### API 키 관리
```yaml
# 환경 변수 (.env)
OPENAI_API_KEY=sk-...
ANTHROPIC_API_KEY=sk-ant-...
GOOGLE_VISION_API_KEY=AIza...

# 키 로테이션 정책
- 90일 주기로 자동 갱신
- 사용량 모니터링
- 이상 패턴 감지 시 자동 차단
```

## 성능 최적화

### 캐싱 전략
```javascript
// Redis 캐싱
const cacheMiddleware = async (req, res, next) => {
  const cacheKey = `preprocess:${crypto.createHash('md5')
    .update(req.body.text)
    .digest('hex')}`;
  
  const cached = await redis.get(cacheKey);
  if (cached) {
    return res.json(JSON.parse(cached));
  }
  
  // 원본 핸들러 실행 후 캐시 저장
  res.sendResponse = res.json;
  res.json = (body) => {
    redis.setex(cacheKey, 3600, JSON.stringify(body));
    res.sendResponse(body);
  };
  
  next();
};
```

### 작업 큐 처리
```javascript
// Bull 큐를 사용한 비동기 처리
const aiQueue = new Bull('ai-processing', {
  redis: { port: 6379, host: 'localhost' }
});

// 작업 추가
aiQueue.add('generate-text', {
  prompt: userPrompt,
  userId: userId,
  model: 'gpt-4'
});

// 작업 처리
aiQueue.process('generate-text', async (job) => {
  const { prompt, model } = job.data;
  const result = await callAIService(prompt, model);
  
  // 결과를 웹소켓으로 실시간 전송
  io.to(job.data.userId).emit('ai-response', result);
  
  return result;
});
```

## 에러 처리

### 글로벌 에러 핸들러
```javascript
// 에러 타입 정의
class APIError extends Error {
  constructor(statusCode, message, code) {
    super(message);
    this.statusCode = statusCode;
    this.code = code;
  }
}

// 에러 처리 미들웨어
app.use((err, req, res, next) => {
  const { statusCode = 500, message, code } = err;
  
  logger.error({
    error: message,
    code,
    url: req.url,
    method: req.method,
    ip: req.ip,
    userId: req.user?.userId
  });
  
  res.status(statusCode).json({
    error: {
      message: statusCode === 500 ? 'Internal server error' : message,
      code: code || 'UNKNOWN_ERROR'
    }
  });
});
```

### 서비스별 에러 처리
```javascript
// AI 서비스 에러
const AI_ERRORS = {
  RATE_LIMIT: { code: 'AI_RATE_LIMIT', message: 'AI 요청 한도 초과' },
  INVALID_MODEL: { code: 'AI_INVALID_MODEL', message: '잘못된 AI 모델' },
  CONTEXT_TOO_LONG: { code: 'AI_CONTEXT_OVERFLOW', message: '컨텍스트 길이 초과' }
};

// OCR 에러
const OCR_ERRORS = {
  IMAGE_TOO_LARGE: { code: 'OCR_IMAGE_SIZE', message: '이미지 크기 초과 (최대 10MB)' },
  INVALID_FORMAT: { code: 'OCR_FORMAT', message: '지원하지 않는 이미지 형식' },
  NO_TEXT_FOUND: { code: 'OCR_NO_TEXT', message: '텍스트를 찾을 수 없음' }
};
```

## 모니터링 및 로깅

### 로깅 설정
```javascript
// Winston 로거 설정
const logger = winston.createLogger({
  level: 'info',
  format: winston.format.json(),
  transports: [
    new winston.transports.File({ filename: 'error.log', level: 'error' }),
    new winston.transports.File({ filename: 'combined.log' })
  ]
});

// API 요청 로깅
app.use((req, res, next) => {
  const start = Date.now();
  
  res.on('finish', () => {
    logger.info({
      method: req.method,
      url: req.url,
      status: res.statusCode,
      duration: Date.now() - start,
      userId: req.user?.userId
    });
  });
  
  next();
});
```

### 메트릭 수집
```javascript
// Prometheus 메트릭
const prometheus = require('prom-client');

const httpRequestDuration = new prometheus.Histogram({
  name: 'http_request_duration_seconds',
  help: 'Duration of HTTP requests in seconds',
  labelNames: ['method', 'route', 'status']
});

const aiRequestCount = new prometheus.Counter({
  name: 'ai_requests_total',
  help: 'Total number of AI requests',
  labelNames: ['model', 'status']
});
```

## 배포 및 확장

### Docker 컨테이너화
```dockerfile
# Dockerfile
FROM node:18-alpine

WORKDIR /app

COPY package*.json ./
RUN npm ci --only=production

COPY . .

EXPOSE 3000

CMD ["node", "src/server.js"]
```

### 수평 확장 전략
```yaml
# Kubernetes 배포
apiVersion: apps/v1
kind: Deployment
metadata:
  name: ghostype-api
spec:
  replicas: 3
  strategy:
    type: RollingUpdate
    rollingUpdate:
      maxSurge: 1
      maxUnavailable: 0
  template:
    spec:
      containers:
      - name: api
        image: ghostype/api:latest
        env:
        - name: NODE_ENV
          value: "production"
        resources:
          requests:
            memory: "256Mi"
            cpu: "250m"
          limits:
            memory: "512Mi"
            cpu: "500m"
```

## 관련 문서
- **[frontend.md](frontend.md)**: Flutter 앱 API 클라이언트
- **[../CLAUDE.md](../CLAUDE.md)**: 전체 시스템 아키텍처
- **[README.md](README.md)**: 프로젝트 개요
