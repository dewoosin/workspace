import { Router } from 'express';
import { Logger } from '../../logging/Logger';
import { strictRateLimiter } from '../middleware/rate-limiter.middleware';
import { container } from 'tsyringe';
import { AuthController } from '../controllers/auth.controller';

const logger = new Logger('CommonRoutes');

export const commonRouter = Router();

/**
 * Common API Routes
 * 
 * These routes provide shared functionality across all client applications:
 * - Health checks and API information
 * - Configuration endpoints
 * - System-wide utilities
 */

// API Root - General API Information
commonRouter.get('/', (req, res) => {
  res.json({
    name: 'Paperly API',
    version: '1.0.0',
    status: 'running',
    timestamp: new Date().toISOString(),
    endpoints: {
      mobile: '/api/v1/mobile/',
      writer: '/api/v1/writer/',
      admin: '/api/v1/admin/',
      common: '/api/v1/common/'
    }
  });
});

// Health Check
commonRouter.get('/health', (req, res) => {
  res.json({
    success: true,
    data: {
      status: 'healthy',
      timestamp: new Date().toISOString(),
      version: '1.0.0',
      environment: process.env.NODE_ENV || 'development'
    },
    message: 'API 서버가 정상 작동 중입니다'
  });
});

// Readiness Probe
commonRouter.get('/health/ready', (req, res) => {
  // TODO: Add database and Redis connectivity checks
  res.json({
    success: true,
    data: {
      status: 'ready',
      timestamp: new Date().toISOString(),
      services: {
        database: 'connected',
        redis: 'connected',
        email: 'available'
      }
    }
  });
});

// Liveness Probe
commonRouter.get('/health/live', (req, res) => {
  res.json({
    success: true,
    data: {
      status: 'alive',
      timestamp: new Date().toISOString(),
      uptime: process.uptime(),
      memory: process.memoryUsage()
    }
  });
});

// API Configuration
commonRouter.get('/config', (req, res) => {
  res.json({
    success: true,
    data: {
      features: {
        registration: true,
        emailVerification: true,
        socialAuth: false,
        push_notifications: true
      },
      limits: {
        maxArticleLength: 50000,
        maxBioLength: 500,
        maxUsernameLength: 30,
        uploadMaxSize: '5MB'
      },
      auth: {
        accessTokenExpiry: '15m',
        refreshTokenExpiry: '7d',
        passwordMinLength: 8,
        passwordRequireSpecialChar: true
      },
      rateLimit: {
        auth: { requests: 5, window: '15m' },
        api: { requests: 100, window: '15m' }
      }
    }
  });
});

// System Information (Development Only)
if (process.env.NODE_ENV === 'development') {
  commonRouter.get('/system/info', (req, res) => {
    res.json({
      success: true,
      data: {
        nodeVersion: process.version,
        platform: process.platform,
        architecture: process.arch,
        memory: process.memoryUsage(),
        uptime: process.uptime(),
        environment: process.env.NODE_ENV,
        timezone: Intl.DateTimeFormat().resolvedOptions().timeZone
      }
    });
  });
}

// Legacy Authentication Routes (Deprecated)
// These maintain backward compatibility for existing clients
const setupLegacyAuthRoutes = () => {
  logger.warn('Loading legacy auth routes for backward compatibility - these will be deprecated');
  
  try {
    const authController = container.resolve(AuthController);
    
    commonRouter.use('/auth', strictRateLimiter, (req, res, next) => {
      logger.warn('Legacy /auth endpoint used - please migrate to client-specific endpoints', {
        ip: req.ip,
        userAgent: req.get('User-Agent'),
        endpoint: req.originalUrl
      });
      
      // Add deprecation header
      res.set('X-API-Deprecated', 'true');
      res.set('X-API-Migration', 'Use /api/v1/mobile/auth, /api/v1/writer/auth, or /api/v1/admin/auth');
      
      authController.router(req, res, next);
    });
    
    logger.info('Legacy auth routes loaded successfully');
  } catch (error) {
    logger.error('Failed to load legacy auth routes:', error);
  }
};

// Load legacy routes
setupLegacyAuthRoutes();

logger.info('Common routes initialized: health checks, config, legacy auth');