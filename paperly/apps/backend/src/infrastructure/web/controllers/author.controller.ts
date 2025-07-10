import { Router, Request, Response } from 'express';
import { injectable, inject } from 'tsyringe';
import { Logger } from '../../logging/Logger';
import { authMiddleware } from '../middleware/auth.middleware';
import { asyncHandler } from '../middlewares/async.middleware';

interface AuthRequest extends Request {
  user?: {
    id: string;
    email: string;
    roles?: string[];
  };
}

@injectable()
export class AuthorController {
  private readonly logger = new Logger('AuthorController');
  public readonly router = Router();

  constructor() {
    this.initializeRoutes();
  }

  private initializeRoutes(): void {
    // Public author endpoints
    this.router.get('/', asyncHandler(this.searchAuthors.bind(this)));
    this.router.get('/trending', asyncHandler(this.getTrendingAuthors.bind(this)));
    this.router.get('/recommended', authMiddleware, asyncHandler(this.getRecommendedAuthors.bind(this)));
    this.router.get('/:authorId', asyncHandler(this.getAuthorDetails.bind(this)));
    this.router.get('/:authorId/stats', asyncHandler(this.getAuthorStats.bind(this)));

    // Follow-related endpoints (require authentication)
    this.router.post('/:authorId/follow', authMiddleware, asyncHandler(this.followAuthor.bind(this)));
    this.router.delete('/:authorId/follow', authMiddleware, asyncHandler(this.unfollowAuthor.bind(this)));
    this.router.get('/:authorId/follow-status', authMiddleware, asyncHandler(this.getFollowStatus.bind(this)));
    this.router.post('/follow-status', authMiddleware, asyncHandler(this.batchGetFollowStatus.bind(this)));

    this.logger.info('Author routes initialized');
  }

  async searchAuthors(req: Request, res: Response): Promise<void> {
    const {
      query = '',
      specialties = '',
      verified_only = false,
      sort_by = 'name',
      sort_order = 'asc',
      page = 1,
      limit = 20
    } = req.query;

    // Mock response for now
    res.json({
      success: true,
      data: {
        authors: [
          {
            id: 'author-1',
            name: 'John Doe',
            username: 'johndoe',
            avatar: 'https://example.com/avatar1.jpg',
            bio: 'Tech writer and software engineer',
            specialties: ['Technology', 'Programming'],
            isVerified: true,
            followerCount: 1250,
            articleCount: 45,
            isFollowing: false
          },
          {
            id: 'author-2',
            name: 'Jane Smith',
            username: 'janesmith',
            avatar: 'https://example.com/avatar2.jpg',
            bio: 'UX designer and product strategist',
            specialties: ['Design', 'Product'],
            isVerified: false,
            followerCount: 890,
            articleCount: 32,
            isFollowing: false
          }
        ],
        pagination: {
          total: 2,
          page: parseInt(page as string),
          limit: parseInt(limit as string),
          totalPages: 1
        }
      }
    });
  }

  async getTrendingAuthors(req: Request, res: Response): Promise<void> {
    const limit = parseInt(req.query.limit as string) || 10;

    res.json({
      success: true,
      data: {
        authors: [
          {
            id: 'trending-author-1',
            name: 'Alex Johnson',
            username: 'alexj',
            avatar: 'https://example.com/avatar3.jpg',
            bio: 'Trending tech influencer',
            specialties: ['AI', 'Machine Learning'],
            isVerified: true,
            followerCount: 5000,
            articleCount: 78,
            weeklyGrowth: 15.5,
            isFollowing: false
          }
        ]
      }
    });
  }

  async getRecommendedAuthors(req: AuthRequest, res: Response): Promise<void> {
    const limit = parseInt(req.query.limit as string) || 5;

    res.json({
      success: true,
      data: {
        authors: [
          {
            id: 'recommended-author-1',
            name: 'Sarah Wilson',
            username: 'sarahw',
            avatar: 'https://example.com/avatar4.jpg',
            bio: 'Recommended based on your interests',
            specialties: ['Business', 'Strategy'],
            isVerified: true,
            followerCount: 3200,
            articleCount: 56,
            matchScore: 0.85,
            isFollowing: false
          }
        ],
        recommendationReason: 'Based on your reading history and interests'
      }
    });
  }

  async getAuthorDetails(req: Request, res: Response): Promise<void> {
    const { authorId } = req.params;

    res.json({
      success: true,
      data: {
        author: {
          id: authorId,
          name: 'John Doe',
          username: 'johndoe',
          avatar: 'https://example.com/avatar1.jpg',
          coverImage: 'https://example.com/cover1.jpg',
          bio: 'Experienced software engineer and technical writer with 10+ years in the industry.',
          specialties: ['Technology', 'Programming', 'Software Engineering'],
          isVerified: true,
          followerCount: 1250,
          followingCount: 180,
          articleCount: 45,
          totalViews: 125000,
          joinedAt: '2022-01-15T00:00:00.000Z',
          socialLinks: {
            twitter: 'https://twitter.com/johndoe',
            linkedin: 'https://linkedin.com/in/johndoe',
            website: 'https://johndoe.dev'
          },
          isFollowing: false
        }
      }
    });
  }

  async getAuthorStats(req: Request, res: Response): Promise<void> {
    const { authorId } = req.params;

    res.json({
      success: true,
      data: {
        stats: {
          totalArticles: 45,
          totalViews: 125000,
          totalLikes: 3200,
          followerCount: 1250,
          averageReadingTime: 6.5,
          publishingFrequency: 'Weekly',
          topCategories: [
            { name: 'Technology', percentage: 60 },
            { name: 'Programming', percentage: 30 },
            { name: 'Career', percentage: 10 }
          ],
          recentGrowth: {
            followers: { count: 50, percentage: 4.2 },
            views: { count: 2500, percentage: 2.0 }
          }
        }
      }
    });
  }

  async followAuthor(req: AuthRequest, res: Response): Promise<void> {
    if (!req.user) {
      res.status(401).json({
        success: false,
        error: {
          code: 'UNAUTHORIZED',
          message: 'Authentication required'
        }
      });
      return;
    }

    const { authorId } = req.params;

    // Mock follow logic
    res.json({
      success: true,
      data: {
        isFollowing: true,
        followerCount: 1251,
        message: 'Successfully followed author'
      }
    });
  }

  async unfollowAuthor(req: AuthRequest, res: Response): Promise<void> {
    if (!req.user) {
      res.status(401).json({
        success: false,
        error: {
          code: 'UNAUTHORIZED',
          message: 'Authentication required'
        }
      });
      return;
    }

    const { authorId } = req.params;

    // Mock unfollow logic
    res.json({
      success: true,
      data: {
        isFollowing: false,
        followerCount: 1249,
        message: 'Successfully unfollowed author'
      }
    });
  }

  async getFollowStatus(req: AuthRequest, res: Response): Promise<void> {
    if (!req.user) {
      res.status(401).json({
        success: false,
        error: {
          code: 'UNAUTHORIZED',
          message: 'Authentication required'
        }
      });
      return;
    }

    const { authorId } = req.params;

    res.json({
      success: true,
      data: {
        isFollowing: false
      }
    });
  }

  async batchGetFollowStatus(req: AuthRequest, res: Response): Promise<void> {
    if (!req.user) {
      res.status(401).json({
        success: false,
        error: {
          code: 'UNAUTHORIZED',
          message: 'Authentication required'
        }
      });
      return;
    }

    const { authorIds } = req.body;

    if (!authorIds || !Array.isArray(authorIds)) {
      res.status(400).json({
        success: false,
        error: {
          code: 'INVALID_REQUEST',
          message: 'authorIds array is required'
        }
      });
      return;
    }

    // Mock batch follow status
    const followStatus: { [key: string]: boolean } = {};
    authorIds.forEach(id => {
      followStatus[id] = false; // Mock: user not following any authors
    });

    res.json({
      success: true,
      data: followStatus
    });
  }
}