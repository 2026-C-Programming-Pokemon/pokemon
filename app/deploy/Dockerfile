# 라즈베리파이/ARM 보드용 슬림 이미지.
# 빌드: 리포 루트에서
#   docker build -f app/deploy/Dockerfile -t pokemon-llm-proxy:dev .
FROM python:3.12-slim-bookworm

ENV PYTHONDONTWRITEBYTECODE=1 \
    PYTHONUNBUFFERED=1 \
    PIP_NO_CACHE_DIR=1 \
    PIP_DISABLE_PIP_VERSION_CHECK=1

WORKDIR /app

COPY app/requirements.txt /app/app/requirements.txt
RUN pip install -r /app/app/requirements.txt

COPY app /app/app

# 비-루트 유저로 실행.
RUN useradd --create-home --shell /usr/sbin/nologin appuser \
 && chown -R appuser:appuser /app
USER appuser

EXPOSE 8080
CMD ["python", "-m", "app.main"]
