# Docker Deployment Guide

## Overview

This backend application is containerized with Docker, including Node.js, Python, and compiled C++ components.

## Prerequisites

- Docker 20.10 or higher
- Docker Compose 2.0 or higher (optional, for docker-compose usage)

## Quick Start

### Option 1: Using Docker Compose (Recommended for Development)

```bash
# Build and start the container
docker-compose up -d

# View logs
docker-compose logs -f

# Stop the container
docker-compose down
```

### Option 2: Using Docker Commands Directly

```bash
# Build the image
docker build -t timetable-backend:latest .

# Run the container
docker run -d \
  --name timetable-backend \
  -p 5000:5000 \
  --restart unless-stopped \
  timetable-backend:latest

# View logs
docker logs -f timetable-backend

# Stop and remove container
docker stop timetable-backend
docker rm timetable-backend
```

## Production Deployment

### Building for Production

```bash
# Build with a specific tag
docker build -t timetable-backend:v1.0.0 .

# Tag for registry (replace with your registry)
docker tag timetable-backend:v1.0.0 yourusername/timetable-backend:v1.0.0
docker tag timetable-backend:v1.0.0 yourusername/timetable-backend:latest

# Push to Docker Hub
docker push yourusername/timetable-backend:v1.0.0
docker push yourusername/timetable-backend:latest
```

### Deploy from Registry

```bash
# Pull and run from Docker Hub
docker pull yourusername/timetable-backend:latest
docker run -d \
  --name timetable-backend \
  -p 5000:5000 \
  --restart unless-stopped \
  yourusername/timetable-backend:latest
```

## Deployment Platforms

### Docker Hub

1. Build and tag your image
2. Push to Docker Hub
3. Pull and run on your server

### AWS ECS/Fargate

1. Push image to Amazon ECR
2. Create ECS task definition
3. Deploy as ECS service

### Google Cloud Run

```bash
# Build and push to Google Container Registry
gcloud builds submit --tag gcr.io/YOUR_PROJECT_ID/timetable-backend

# Deploy to Cloud Run
gcloud run deploy timetable-backend \
  --image gcr.io/YOUR_PROJECT_ID/timetable-backend \
  --platform managed \
  --region us-central1 \
  --allow-unauthenticated \
  --port 5000
```

### Azure Container Instances

```bash
# Push to Azure Container Registry
az acr build --registry YOUR_REGISTRY --image timetable-backend:latest .

# Deploy to ACI
az container create \
  --resource-group YOUR_RESOURCE_GROUP \
  --name timetable-backend \
  --image YOUR_REGISTRY.azurecr.io/timetable-backend:latest \
  --cpu 1 --memory 1 \
  --ports 5000
```

### DigitalOcean App Platform

1. Push image to DigitalOcean Container Registry
2. Create app from registry image
3. Configure port 5000

### Render

1. Connect your GitHub repository
2. Select "Docker" as environment
3. Render will auto-detect Dockerfile

## Environment Variables

Create a `.env` file for local development (see `.env.example`):

```bash
PORT=5000
NODE_ENV=production
```

Pass environment variables when running:

```bash
docker run -d \
  --name timetable-backend \
  -p 5000:5000 \
  -e PORT=5000 \
  -e NODE_ENV=production \
  timetable-backend:latest
```

## Health Check

The container includes a health check endpoint:

```bash
# Check health
curl http://localhost:5000/health
```

Docker will automatically monitor this endpoint.

## Troubleshooting

### View container logs

```bash
docker logs timetable-backend
docker logs -f timetable-backend  # Follow logs
```

### Enter container shell

```bash
docker exec -it timetable-backend /bin/bash
```

### Check container status

```bash
docker ps -a
docker inspect timetable-backend
```

### Rebuild after changes

```bash
docker-compose down
docker-compose build --no-cache
docker-compose up -d
```

## Security Notes

- The container runs as a non-root user (appuser)
- Only production dependencies are included
- Sensitive files are excluded via .dockerignore
- Health checks ensure container reliability

## Image Size Optimization

The Dockerfile uses multi-stage builds to minimize image size:

- Builder stage: Compiles C++ and installs all dependencies
- Production stage: Contains only runtime dependencies

Expected image size: ~400-500MB

## Port Configuration

- Default port: 5000
- Configure via PORT environment variable
- Update EXPOSE in Dockerfile if changing default

## Support

For issues or questions, please refer to the main project repository.
