# Multi-stage build for optimized image size
FROM nikolaik/python-nodejs:python3.12-nodejs22 AS builder

RUN apt-get update && apt-get install -y g++ make && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY package*.json ./
COPY requirements.txt ./

RUN npm ci && npm prune --production

RUN python -m venv /opt/venv \
 && . /opt/venv/bin/activate \
 && pip install --no-cache-dir -r requirements.txt

COPY . .

RUN cd cpp-files && \
    g++ -std=c++17 -O2 -o generator generator.cpp && \
    g++ -std=c++17 -O2 -o generator2 generator2.cpp && \
    g++ -std=c++17 -O2 -o checker checker.cpp && \
    g++ -std=c++17 -O2 -o checker2 checker2.cpp && \
    g++ -std=c++17 -O2 -o slot_suggester slot_suggester.cpp

FROM nikolaik/python-nodejs:python3.12-nodejs22

RUN useradd -m -u 1001 appuser
WORKDIR /app

COPY --from=builder --chown=appuser:appuser /opt/venv /opt/venv
COPY --from=builder --chown=appuser:appuser /app /app

ENV PATH="/opt/venv/bin:$PATH" \
    PYTHONDONTWRITEBYTECODE=1 \
    PYTHONUNBUFFERED=1 \
    NODE_ENV=production \
    PORT=5000

USER appuser
EXPOSE 5000

HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
  CMD node -e "require('http').get(`http://localhost:${process.env.PORT}/health`, r => {process.exit(r.statusCode === 200 ? 0 : 1)})"

CMD ["node", "server.js"]