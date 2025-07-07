/// Paperly Backend - Domain Layer Index
/// 
/// 도메인 계층의 모든 Public API를 export하는 중앙 진입점입니다.
/// 다른 계층(Application, Infrastructure)에서 이 파일을 통해 import해야 합니다.
/// 
/// 도메인 계층 구성:
/// - Entities: 도메인 핵심 비즈니스 객체들
/// - Value Objects: 값 객체들 (Email, Password, UserId 등)
/// - Repositories: 도메인 저장소 인터페이스들
/// - Services: 복잡한 도메인 로직을 처리하는 도메인 서비스들
/// - Types: 도메인 관련 타입 및 인터페이스 정의

// =============================================================================
// 🏗️ Domain Entities (도메인 엔티티)
// =============================================================================
export * from './entities';

// =============================================================================
// 💎 Value Objects (값 객체들)  
// =============================================================================
export * from './value-objects';

// =============================================================================
// 📦 Repositories (저장소 인터페이스)
// =============================================================================
export * from './repositories';

// =============================================================================
// ⚙️ Domain Services (도메인 서비스)
// =============================================================================
export * from './services';

// =============================================================================
// 📝 Auth Types (인증 관련 타입들) - auth.types.ts에서만 export
// =============================================================================
// Note: auth.types.ts의 타입들만 export하여 중복 방지
export { 
  Gender, 
  DeviceInfo, 
  LoginAttempt,
  RegisterRequest,
  LoginRequest,
  RefreshTokenRequest,
  AuthResponse,
  UserInfo,
  TokenPair,
  JwtPayload,
  RefreshTokenModel,
  EmailVerificationToken,
  PasswordResetToken,
  UserSession,
  AuthContext,
  AuthHeaders
} from './auth/auth.types';