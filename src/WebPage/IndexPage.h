#ifndef WEB_PAGE_H
#define WEB_PAGE_H

// ==============================================================================
// OPTIMIZED SELF-CONTAINED HTML5 / CSS / JAVASCRIPT DASHBOARD FOR ESP32
// - Fits directly into Flash memory (PROGMEM) to save dynamic RAM.
// - Zero external CDN dependencies (works completely offline / AP / STA mode).
// - Responsive Slate & Cyan theme matching the Node.js / React Web Dashboard.
// ==============================================================================

const char PAGE_INDEX[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="en" class="dark">
  <head>
    <meta charset="UTF-8" />
    <meta
      name="viewport"
      content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no"
    />
    <title>Floor Management</title>
    <style>
      :root {
        --bg: #0f172a;
        --card: #1e293b;
        --card-hover: #273549;
        --border: #334155;
        --text: #f1f5f9;
        --text-muted: #94a3b8;
        --primary: #06b6d4;
        --primary-hover: #0891b2;
        --success: #22c55e;
        --danger: #ef4444;
        --warning: #f59e0b;
        --shadow:
          0 4px 6px -1px rgba(0, 0, 0, 0.2), 0 2px 4px -2px rgba(0, 0, 0, 0.1);
      }
      html.light {
        --bg: #f8fafc;
        --card: #ffffff;
        --card-hover: #f1f5f9;
        --border: #e2e8f0;
        --text: #0f172a;
        --text-muted: #64748b;
        --shadow:
          0 4px 6px -1px rgba(0, 0, 0, 0.05), 0 2px 4px -2px rgba(0, 0, 0, 0.05);
      }
      * {
        box-sizing: border-box;
        margin: 0;
        padding: 0;
        font-family:
          -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
        -webkit-tap-highlight-color: transparent;
      }
      body {
        background: var(--bg);
        color: var(--text);
        min-height: 100vh;
        padding-bottom: 2rem;
        transition:
          background 0.2s,
          color 0.2s;
      }
      .nav {
        position: sticky;
        top: 0;
        z-index: 50;
        background: rgba(15, 23, 42, 0.85);
        backdrop-filter: blur(8px);
        border-bottom: 1px solid var(--border);
        height: 60px;
        display: flex;
        align-items: center;
        justify-content: space-between;
        padding: 0 1rem;
      }
      html.light .nav {
        background: rgba(255, 255, 255, 0.85);
      }
      .brand {
        display: flex;
        align-items: center;
        gap: 0.6rem;
        font-weight: 700;
        font-size: 1.1rem;
        color: var(--primary);
        letter-spacing: 0.5px;
      }
      .logo-box {
        width: 34px;
        height: 34px;
        border-radius: 8px;
        background: linear-gradient(135deg, #06b6d4, #0891b2);
        display: flex;
        align-items: center;
        justify-content: center;
        color: #fff;
      }
      .nav-actions {
        display: flex;
        align-items: center;
        gap: 0.5rem;
      }
      .status-badge {
        display: flex;
        align-items: center;
        gap: 0.4rem;
        padding: 0.3rem 0.6rem;
        border-radius: 999px;
        font-size: 0.75rem;
        font-weight: 600;
        background: rgba(34, 197, 94, 0.1);
        border: 1px solid rgba(34, 197, 94, 0.25);
        color: var(--success);
      }
      .status-dot {
        width: 7px;
        height: 7px;
        border-radius: 50%;
        background: var(--success);
        animation: pulse 2s infinite;
      }
      @keyframes pulse {
        0%,
        100% {
          opacity: 1;
          transform: scale(1);
        }
        50% {
          opacity: 0.4;
          transform: scale(0.85);
        }
      }
      .icon-btn {
        background: transparent;
        border: 1px solid var(--border);
        color: var(--text-muted);
        width: 34px;
        height: 34px;
        border-radius: 8px;
        display: flex;
        align-items: center;
        justify-content: center;
        cursor: pointer;
        transition: 0.15s;
      }
      .icon-btn:hover {
        background: var(--card-hover);
        color: var(--text);
      }
      .container {
        max-width: 1100px;
        margin: 0 auto;
        padding: 1rem;
      }
      .emergency-bar {
        background: rgba(239, 68, 68, 0.08);
        border: 1px solid rgba(239, 68, 68, 0.25);
        border-radius: 12px;
        padding: 0.75rem 1rem;
        display: flex;
        align-items: center;
        justify-content: space-between;
        margin-bottom: 1.25rem;
        flex-wrap: wrap;
        gap: 0.5rem;
      }
      .emergency-title {
        display: flex;
        align-items: center;
        gap: 0.5rem;
        font-size: 0.85rem;
        font-weight: 700;
        color: var(--danger);
        text-transform: uppercase;
        letter-spacing: 0.5px;
      }
      .btn-emergency {
        background: #ef4444;
        color: #fff;
        border: none;
        padding: 0.45rem 0.9rem;
        border-radius: 8px;
        font-size: 0.75rem;
        font-weight: 700;
        cursor: pointer;
        transition: 0.15s;
        display: inline-flex;
        align-items: center;
        gap: 0.4rem;
      }
      .btn-emergency:hover {
        background: #dc2626;
      }
      .btn-clear {
        background: transparent;
        border: 1px solid var(--border);
        color: var(--text-muted);
        padding: 0.45rem 0.75rem;
        border-radius: 8px;
        font-size: 0.75rem;
        font-weight: 600;
        cursor: pointer;
      }
      .btn-clear:hover {
        background: var(--card-hover);
        color: var(--text);
      }
      .section-header {
        display: flex;
        align-items: center;
        justify-content: space-between;
        margin: 1.5rem 0 0.75rem;
      }
      .section-title {
        font-size: 0.85rem;
        font-weight: 700;
        color: var(--text-muted);
        text-transform: uppercase;
        letter-spacing: 0.5px;
      }
      .grid {
        display: grid;
        grid-template-columns: repeat(auto-fill, minmax(280px, 1fr));
        gap: 1rem;
      }
      .card {
        background: var(--card);
        border: 1px solid var(--border);
        border-radius: 14px;
        padding: 1.1rem;
        box-shadow: var(--shadow);
        transition:
          transform 0.15s,
          border-color 0.15s;
        display: flex;
        flex-direction: column;
        justify-content: space-between;
      }
      .card:hover {
        border-color: rgba(6, 182, 212, 0.4);
      }
      .card-top {
        display: flex;
        align-items: flex-start;
        justify-content: space-between;
        margin-bottom: 1rem;
      }
      .card-meta {
        display: flex;
        align-items: center;
        gap: 0.75rem;
      }
      .card-icon {
        width: 40px;
        height: 40px;
        border-radius: 10px;
        background: rgba(6, 182, 212, 0.1);
        border: 1px solid rgba(6, 182, 212, 0.2);
        display: flex;
        align-items: center;
        justify-content: center;
        color: var(--primary);
        transition: 0.2s;
      }
      .card-icon.active {
        background: rgba(34, 197, 94, 0.15);
        border-color: rgba(34, 197, 94, 0.4);
        color: var(--success);
      }
      .card-icon.danger {
        background: rgba(239, 68, 68, 0.15);
        border-color: rgba(239, 68, 68, 0.4);
        color: var(--danger);
      }
      .card-name {
        font-weight: 600;
        font-size: 0.95rem;
        margin-bottom: 0.2rem;
      }
      .card-sub {
        font-size: 0.75rem;
        color: var(--text-muted);
      }
      .pill {
        font-size: 0.7rem;
        font-weight: 700;
        padding: 0.25rem 0.55rem;
        border-radius: 999px;
        text-transform: uppercase;
      }
      .pill.on {
        background: rgba(34, 197, 94, 0.15);
        color: var(--success);
        border: 1px solid rgba(34, 197, 94, 0.3);
      }
      .pill.off {
        background: rgba(148, 163, 184, 0.15);
        color: var(--text-muted);
        border: 1px solid rgba(148, 163, 184, 0.2);
      }
      .pill.alert {
        background: rgba(239, 68, 68, 0.15);
        color: var(--danger);
        border: 1px solid rgba(239, 68, 68, 0.3);
        animation: pulse 1s infinite;
      }
      .card-actions {
        display: flex;
        align-items: center;
        gap: 0.5rem;
      }
      .btn-toggle {
        flex: 1;
        padding: 0.6rem;
        border-radius: 8px;
        border: none;
        font-size: 0.85rem;
        font-weight: 600;
        cursor: pointer;
        transition: 0.15s;
        display: flex;
        align-items: center;
        justify-content: center;
        gap: 0.4rem;
        background: rgba(6, 182, 212, 0.1);
        color: var(--primary);
        border: 1px solid rgba(6, 182, 212, 0.25);
      }
      .btn-toggle:hover {
        background: rgba(6, 182, 212, 0.2);
      }
      .btn-toggle.active {
        background: var(--success);
        color: #fff;
        border-color: var(--success);
      }
      .btn-toggle.active:hover {
        background: #16a34a;
      }
      .btn-timer {
        background: transparent;
        border: 1px solid var(--border);
        color: var(--text-muted);
        padding: 0.6rem;
        border-radius: 8px;
        cursor: pointer;
        display: flex;
        align-items: center;
        justify-content: center;
        width: 38px;
        height: 38px;
      }
      .btn-timer:hover {
        background: var(--card-hover);
        color: var(--text);
      }
      .parking-grid {
        display: grid;
        grid-template-columns: repeat(4, 1fr);
        gap: 0.5rem;
        margin-top: 0.5rem;
      }
      .slot-box {
        background: var(--bg);
        border: 1px solid var(--border);
        border-radius: 8px;
        padding: 0.5rem;
        text-align: center;
      }
      .slot-id {
        font-size: 0.75rem;
        font-weight: 700;
        color: var(--text-muted);
        margin-bottom: 0.2rem;
      }
      .slot-status {
        font-size: 0.7rem;
        font-weight: 600;
        border-radius: 4px;
        padding: 0.15rem 0;
      }
      .slot-status.free {
        color: var(--success);
        background: rgba(34, 197, 94, 0.1);
      }
      .slot-status.occ {
        color: var(--danger);
        background: rgba(239, 68, 68, 0.1);
      }
      .footer {
        margin-top: 2rem;
        padding: 1rem;
        background: var(--card);
        border: 1px solid var(--border);
        border-radius: 12px;
        display: flex;
        flex-wrap: wrap;
        align-items: center;
        justify-content: space-between;
        gap: 0.75rem;
        font-size: 0.75rem;
        color: var(--text-muted);
      }
      .footer-stat {
        display: flex;
        align-items: center;
        gap: 0.4rem;
      }
      .val {
        color: var(--text);
        font-weight: 600;
        font-family: monospace;
      }
      .toast {
        position: fixed;
        bottom: 1.5rem;
        right: 1.5rem;
        background: #0f172a;
        border: 1px solid var(--primary);
        color: #fff;
        padding: 0.75rem 1.25rem;
        border-radius: 10px;
        font-size: 0.85rem;
        font-weight: 500;
        box-shadow: 0 10px 15px -3px rgba(0, 0, 0, 0.3);
        opacity: 0;
        transform: translateY(10px);
        transition: 0.2s;
        z-index: 100;
        pointer-events: none;
      }
      .toast.show {
        opacity: 1;
        transform: translateY(0);
      }
    </style>
  </head>
  <body>
    <nav class="nav">
      <div class="brand">
        <div class="logo-box">
          <svg
            width="20"
            height="20"
            fill="none"
            stroke="currentColor"
            stroke-width="2"
            viewBox="0 0 24 24"
          >
            <rect width="16" height="20" x="4" y="2" rx="2" />
            <path
              d="M9 22v-4h6v4M8 6h.01M16 6h.01M12 6h.01M12 10h.01M12 14h.01M16 10h.01M16 14h.01M8 10h.01M8 14h.01"
            />
          </svg>
        </div>
        <span>FLOOR MGMT</span>
      </div>
      <div class="nav-actions">
        <!-- <div class="status-badge" id="netBadge">
          <div class="status-dot"></div>
          <span id="netText">Online</span>
        </div> -->
        <button class="icon-btn" onclick="toggleTheme()" title="Toggle Theme">
          <svg
            width="18"
            height="18"
            fill="none"
            stroke="currentColor"
            stroke-width="2"
            viewBox="0 0 24 24"
          >
            <circle cx="12" cy="12" r="4" />
            <path
              d="M12 2v2M12 20v2M4.93 4.93l1.41 1.41M17.66 17.66l1.41 1.41M2 12h2M20 12h2M6.34 17.66l-1.41 1.41M19.07 4.93l-1.41 1.41"
            />
          </svg>
        </button>
        <button class="icon-btn" onclick="fetchState()" title="Refresh">
          <svg
            width="18"
            height="18"
            fill="none"
            stroke="currentColor"
            stroke-width="2"
            viewBox="0 0 24 24"
          >
            <path d="M21 12a9 9 0 0 0-9-9 9.75 9.75 0 0 0-6.74 2.74L3 8" />
            <path
              d="M3 3v5h5M3 12a9 9 0 0 0 9 9 9.75 9.75 0 0 0 6.74-2.74L21 16"
            />
            <path d="M16 21h5v-5" />
          </svg>
        </button>
      </div>
    </nav>

    <div class="container">
      <!-- Emergency Protocols -->
      <!-- <div class="emergency-bar">
        <div class="emergency-title">
          <svg
            width="18"
            height="18"
            fill="none"
            stroke="currentColor"
            stroke-width="2"
            viewBox="0 0 24 24"
          >
            <path
              d="m21.73 18-8-14a2 2 0 0 0-3.48 0l-8 14A2 2 0 0 0 4 21h16a2 2 0 0 0 1.73-3Z"
            />
            <line x1="12" y1="9" x2="12" y2="13" />
            <line x1="12" y1="17" x2="12.01" y2="17" />
          </svg>
          <span>Emergency Protocols</span>
        </div>
        <div style="display: flex; gap: 0.5rem">
          <button class="btn-emergency" onclick="setEmergency('evacuate')">
            Evacuate
          </button>
          <button
            class="btn-emergency"
            style="background: #475569"
            onclick="setEmergency('lockdown')"
          >
            Lockdown
          </button>
          <button class="btn-clear" onclick="setEmergency('clear')">
            Clear
          </button>
        </div>
      </div> -->

      <!-- Smart Lighting -->
      <div class="section-header">
        <span class="section-title">Smart Lighting (Relays)</span>
        <div style="display: flex; gap: 0.4rem">
          <button
            class="btn-clear"
            style="padding: 0.25rem 0.5rem"
            onclick="batchLights(true)"
          >
            All ON
          </button>
          <button
            class="btn-clear"
            style="padding: 0.25rem 0.5rem"
            onclick="batchLights(false)"
          >
            All OFF
          </button>
        </div>
      </div>

      <div class="grid">
        <!-- Zone A Light -->
        <div class="card" id="card-light1">
          <div class="card-top">
            <div class="card-meta">
              <div class="card-icon" id="icon-light1">
                <svg
                  width="22"
                  height="22"
                  fill="none"
                  stroke="currentColor"
                  stroke-width="2"
                  viewBox="0 0 24 24"
                >
                  <path
                    d="M15 14c.2-1 .7-1.7 1.5-2.5 1-.9 1.5-2.2 1.5-3.5A6 6 0 0 0 6 8c0 1 .2 2.2 1.5 3.5.7.7 1.3 1.5 1.5 2.5"
                  />
                  <path d="M9 18h6M10 22h4" />
                </svg>
              </div>
              <div>
                <div class="card-name">Zone A Lighting</div>
                <div class="card-sub">Relay 1 • GPIO 2</div>
              </div>
            </div>
            <span class="pill off" id="pill-light1">OFF</span>
          </div>
          <div class="card-actions">
            <button
              class="btn-toggle"
              id="btn-light1"
              onclick="toggleDevice('light1')"
            >
              <svg
                width="16"
                height="16"
                fill="none"
                stroke="currentColor"
                stroke-width="2"
                viewBox="0 0 24 24"
              >
                <path d="M18.36 6.64a9 9 0 1 1-12.73 0M12 2v10" />
              </svg>
              <span id="lbl-light1">Turn ON</span>
            </button>
            <button class="btn-timer" onclick="setTimer('light1')">
              <svg
                width="18"
                height="18"
                fill="none"
                stroke="currentColor"
                stroke-width="2"
                viewBox="0 0 24 24"
              >
                <circle cx="12" cy="12" r="10" />
                <polyline points="12 6 12 12 16 14" />
              </svg>
            </button>
          </div>
        </div>

        <!-- Zone B Light -->
        <div class="card" id="card-light2">
          <div class="card-top">
            <div class="card-meta">
              <div class="card-icon" id="icon-light2">
                <svg
                  width="22"
                  height="22"
                  fill="none"
                  stroke="currentColor"
                  stroke-width="2"
                  viewBox="0 0 24 24"
                >
                  <path
                    d="M15 14c.2-1 .7-1.7 1.5-2.5 1-.9 1.5-2.2 1.5-3.5A6 6 0 0 0 6 8c0 1 .2 2.2 1.5 3.5.7.7 1.3 1.5 1.5 2.5"
                  />
                  <path d="M9 18h6M10 22h4" />
                </svg>
              </div>
              <div>
                <div class="card-name">Zone B Lighting</div>
                <div class="card-sub">Relay 2 • GPIO 4</div>
              </div>
            </div>
            <span class="pill off" id="pill-light2">OFF</span>
          </div>
          <div class="card-actions">
            <button
              class="btn-toggle"
              id="btn-light2"
              onclick="toggleDevice('light2')"
            >
              <svg
                width="16"
                height="16"
                fill="none"
                stroke="currentColor"
                stroke-width="2"
                viewBox="0 0 24 24"
              >
                <path d="M18.36 6.64a9 9 0 1 1-12.73 0M12 2v10" />
              </svg>
              <span id="lbl-light2">Turn ON</span>
            </button>
            <button class="btn-timer" onclick="setTimer('light2')">
              <svg
                width="18"
                height="18"
                fill="none"
                stroke="currentColor"
                stroke-width="2"
                viewBox="0 0 24 24"
              >
                <circle cx="12" cy="12" r="10" />
                <polyline points="12 6 12 12 16 14" />
              </svg>
            </button>
          </div>
        </div>

        <!-- Zone C Light -->
        <div class="card" id="card-light3">
          <div class="card-top">
            <div class="card-meta">
              <div class="card-icon" id="icon-light3">
                <svg
                  width="22"
                  height="22"
                  fill="none"
                  stroke="currentColor"
                  stroke-width="2"
                  viewBox="0 0 24 24"
                >
                  <path
                    d="M15 14c.2-1 .7-1.7 1.5-2.5 1-.9 1.5-2.2 1.5-3.5A6 6 0 0 0 6 8c0 1 .2 2.2 1.5 3.5.7.7 1.3 1.5 1.5 2.5"
                  />
                  <path d="M9 18h6M10 22h4" />
                </svg>
              </div>
              <div>
                <div class="card-name">Zone C Lighting</div>
                <div class="card-sub">Relay 3 • GPIO 5</div>
              </div>
            </div>
            <span class="pill off" id="pill-light3">OFF</span>
          </div>
          <div class="card-actions">
            <button
              class="btn-toggle"
              id="btn-light3"
              onclick="toggleDevice('light3')"
            >
              <svg
                width="16"
                height="16"
                fill="none"
                stroke="currentColor"
                stroke-width="2"
                viewBox="0 0 24 24"
              >
                <path d="M18.36 6.64a9 9 0 1 1-12.73 0M12 2v10" />
              </svg>
              <span id="lbl-light3">Turn ON</span>
            </button>
            <button class="btn-timer" onclick="setTimer('light3')">
              <svg
                width="18"
                height="18"
                fill="none"
                stroke="currentColor"
                stroke-width="2"
                viewBox="0 0 24 24"
              >
                <circle cx="12" cy="12" r="10" />
                <polyline points="12 6 12 12 16 14" />
              </svg>
            </button>
          </div>
        </div>

        <!-- Emergency Party Beacon -->
        <div class="card" id="card-party">
          <div class="card-top">
            <div class="card-meta">
              <div class="card-icon" id="icon-party">
                <svg
                  width="22"
                  height="22"
                  fill="none"
                  stroke="currentColor"
                  stroke-width="2"
                  viewBox="0 0 24 24"
                >
                  <polygon points="13 2 3 14 12 14 11 22 21 10 12 10 13 2" />
                </svg>
              </div>
              <div>
                <div class="card-name">Party / Siren Light</div>
                <div class="card-sub">PWM Beacon • GPIO 23</div>
              </div>
            </div>
            <span class="pill off" id="pill-party">OFF</span>
          </div>
          <div class="card-actions">
            <button
              class="btn-toggle"
              id="btn-party"
              onclick="toggleDevice('party')"
            >
              <span id="lbl-party">Activate</span>
            </button>
          </div>
        </div>
      </div>

      <!-- Entrance & Barrier Controls -->
      <div class="section-header">
        <span class="section-title">Access & Barriers</span>
      </div>
      <div class="grid">
        <!-- Rolling Door -->
        <div class="card" id="card-door">
          <div class="card-top">
            <div class="card-meta">
              <div class="card-icon" id="icon-door">
                <svg
                  width="22"
                  height="22"
                  fill="none"
                  stroke="currentColor"
                  stroke-width="2"
                  viewBox="0 0 24 24"
                >
                  <rect width="18" height="18" x="3" y="3" rx="2" />
                  <path d="M3 9h18M3 15h18M9 3v18M15 3v18" />
                </svg>
              </div>
              <div>
                <div class="card-name">Rolling Door</div>
                <div class="card-sub">L9110S Motor • GPIO 18/19</div>
              </div>
            </div>
            <span class="pill off" id="pill-door">CLOSED</span>
          </div>
          <div class="card-actions">
            <button
              class="btn-toggle"
              id="btn-door-open"
              onclick="controlDoor('open')"
            >
              Open (2s)
            </button>
            <button
              class="btn-toggle"
              id="btn-door-close"
              onclick="controlDoor('close')"
              style="
                background: rgba(239, 68, 68, 0.1);
                color: var(--danger);
                border-color: rgba(239, 68, 68, 0.25);
              "
            >
              Close (2s)
            </button>
          </div>
        </div>

        <!-- Boom Gate -->
        <div class="card" id="card-gate">
          <div class="card-top">
            <div class="card-meta">
              <div class="card-icon" id="icon-gate">
                <svg
                  width="22"
                  height="22"
                  fill="none"
                  stroke="currentColor"
                  stroke-width="2"
                  viewBox="0 0 24 24"
                >
                  <path d="M4 21V9a2 2 0 0 1 2-2h12a2 2 0 0 1 2 2v12M9 13h6" />
                </svg>
              </div>
              <div>
                <div class="card-name">Boom Gate</div>
                <div class="card-sub">SG90 Servo • GPIO 27</div>
              </div>
            </div>
            <span class="pill off" id="pill-gate">CLOSED</span>
          </div>
          <div class="card-actions">
            <button class="btn-toggle" onclick="controlGate(true)">OPEN</button>
            <button class="btn-toggle" onclick="controlGate(false)">
              CLOSE
            </button>
          </div>
        </div>

        <!-- Parking Slot Monitor -->
        <div class="card">
          <div class="card-top">
            <div class="card-meta">
              <div class="card-icon">
                <svg
                  width="22"
                  height="22"
                  fill="none"
                  stroke="currentColor"
                  stroke-width="2"
                  viewBox="0 0 24 24"
                >
                  <path
                    d="M19 17h2c.6 0 1-.4 1-1v-3c0-.9-.7-1.7-1.5-1.9C18.7 10.6 16 10 16 10s-1.3-1.4-2.2-2.3c-.5-.4-1.1-.7-1.8-.7H5c-.6 0-1.1.4-1.4.9l-1.4 2.9A3.7 3.7 0 0 0 2 12v4c0 .6.4 1 1 1h2"
                  />
                  <circle cx="7" cy="17" r="2" />
                  <circle cx="17" cy="17" r="2" />
                </svg>
              </div>
              <div>
                <div class="card-name">Floor Bays Monitor</div>
              </div>
            </div>
            <span class="pill on" id="free-slots-badge">-- FREE</span>
          </div>
          <div class="parking-grid">
            <div class="slot-box">
              <div class="slot-id">Bay A1</div>
              <div class="slot-status free" id="slot-1">FREE</div>
            </div>
            <div class="slot-box">
              <div class="slot-id">Bay A2</div>
              <div class="slot-status free" id="slot-2">FREE</div>
            </div>
            <div class="slot-box">
              <div class="slot-id">Bay A3</div>
              <div class="slot-status free" id="slot-3">FREE</div>
            </div>
            <div class="slot-box">
              <div class="slot-id">Bay A4</div>
              <div class="slot-status free" id="slot-4">FREE</div>
            </div>
          </div>
        </div>
      </div>

      <!-- Diagnostic Footer -->
      <!-- <div class="footer">
      <div class="footer-stat">IP: <span class="val" id="stat-ip">192.168.1.150</span></div>
      <div class="footer-stat">Free Heap: <span class="val" id="stat-heap">218 KB</span></div>
      <div class="footer-stat">WiFi RSSI: <span class="val" id="stat-rssi">-54 dBm</span></div>
      <div class="footer-stat">Uptime: <span class="val" id="stat-uptime">0h 42m</span></div>
      <div class="footer-stat">Mode: <span class="val" style="color:var(--primary);">ESP32 Direct</span></div>
    </div> -->
    </div>

    <div class="toast" id="toastMsg">Action executed</div>

    <script>
      const state = {
        light1: false,
        light2: false,
        light3: false,
        party: false,
        door: "closed",
        gate: false,
        emergency: "normal",
      };
      const isESP32Host =
        window.location.protocol.startsWith("http") &&
        window.location.port !== "5173";

      function showToast(msg) {
        const t = document.getElementById("toastMsg");
        t.textContent = msg;
        t.classList.add("show");
        setTimeout(() => t.classList.remove("show"), 2200);
      }

      async function toggleDevice(id) {
        state[id] = !state[id];
        renderUI();
        if (isESP32Host) {
          try {
            await fetch(`/api/control?id=${id}&state=${state[id] ? 1 : 0}`);
          } catch (e) {}
        }
        showToast(`${id.toUpperCase()} turned ${state[id] ? "ON" : "OFF"}`);
      }

      async function batchLights(on) {
        state.light1 = on;
        state.light2 = on;
        state.light3 = on;
        renderUI();
        if (isESP32Host) {
          try {
            await fetch(`/api/control?id=all_lights&state=${on ? 1 : 0}`);
          } catch (e) {}
        }
        showToast(`All Lights turned ${on ? "ON" : "OFF"}`);
      }

      async function controlDoor(action) {
        if (state.door === "opening" || state.door === "closing") {
          showToast("Door is currently moving! Wait...");
          return;
        }
        state.door = action === "open" ? "opening" : "closing";
        renderUI();
        showToast(`Rolling Door ${state.door}... (2s transit)`);

        if (isESP32Host) {
          try {
            await fetch(`/api/control?id=door&action=${action}`);
          } catch (e) {}
        }

        setTimeout(() => {
          state.door = action === "open" ? "open" : "closed";
          renderUI();
          showToast(`Rolling Door is now ${state.door.toUpperCase()}`);
        }, 2000);
      }

      async function controlGate(open) {
        state.gate = open;
        renderUI();
        if (isESP32Host) {
          try {
            await fetch(
              `/api/control?id=gate&action=${open ? "open" : "close"}`,
            );
          } catch (e) {}
        }
        showToast(`Boom Gate ${open ? "Raised to 90°" : "Lowered to 0°"}`);
      }

      async function setEmergency(mode) {
        state.emergency = mode;
        if (mode === "evacuate") {
          state.light1 = true;
          state.light2 = true;
          state.light3 = true;
          state.party = true;
          state.door = "open";
          state.gate = true;
          showToast("⚠️ EMERGENCY EVACUATION TRIGGERED!");
        } else if (mode === "lockdown") {
          state.door = "closed";
          state.gate = false;
          showToast("🚨 MASTER LOCKDOWN ACTIVE!");
        } else {
          state.party = false;
          showToast("Emergency Cleared — Normal Mode");
        }
        renderUI();
        if (isESP32Host) {
          try {
            await fetch(`/api/emergency?mode=${mode}`);
          } catch (e) {}
        }
      }

      function setTimer(id) {
        const mins = prompt("Enter auto-off countdown timer (minutes):", "15");
        if (mins && !isNaN(mins)) {
          showToast(`Auto-off timer set for ${mins} minutes`);
          if (isESP32Host) {
            fetch(`/api/control?id=${id}&timer=${mins}`);
          }
        }
      }

      function renderUI() {
        const p1 = document.getElementById("pill-light1");
        const b1 = document.getElementById("btn-light1");
        const i1 = document.getElementById("icon-light1");
        p1.className = state.light1 ? "pill on" : "pill off";
        p1.textContent = state.light1 ? "ON" : "OFF";
        b1.className = state.light1 ? "btn-toggle active" : "btn-toggle";
        document.getElementById("lbl-light1").textContent = state.light1
          ? "Turn OFF"
          : "Turn ON";
        i1.className = state.light1 ? "card-icon active" : "card-icon";

        const p2 = document.getElementById("pill-light2");
        const b2 = document.getElementById("btn-light2");
        const i2 = document.getElementById("icon-light2");
        p2.className = state.light2 ? "pill on" : "pill off";
        p2.textContent = state.light2 ? "ON" : "OFF";
        b2.className = state.light2 ? "btn-toggle active" : "btn-toggle";
        document.getElementById("lbl-light2").textContent = state.light2
          ? "Turn OFF"
          : "Turn ON";
        i2.className = state.light2 ? "card-icon active" : "card-icon";

        const p3 = document.getElementById("pill-light3");
        const b3 = document.getElementById("btn-light3");
        const i3 = document.getElementById("icon-light3");
        if (p3) {
          p3.className = state.light3 ? "pill on" : "pill off";
          p3.textContent = state.light3 ? "ON" : "OFF";
          b3.className = state.light3 ? "btn-toggle active" : "btn-toggle";
          document.getElementById("lbl-light3").textContent = state.light3
            ? "Turn OFF"
            : "Turn ON";
          i3.className = state.light3 ? "card-icon active" : "card-icon";
        }

        const pp = document.getElementById("pill-party");
        const bp = document.getElementById("btn-party");
        const ip = document.getElementById("icon-party");
        pp.className = state.party ? "pill alert" : "pill off";
        pp.textContent = state.party ? "STROBE" : "OFF";
        bp.className = state.party ? "btn-toggle active" : "btn-toggle";
        document.getElementById("lbl-party").textContent = state.party
          ? "Deactivate"
          : "Activate";
        ip.className = state.party ? "card-icon danger" : "card-icon";

        const pd = document.getElementById("pill-door");
        const idIcon = document.getElementById("icon-door");
        pd.textContent = state.door.toUpperCase();
        if (state.door === "open") {
          pd.className = "pill on";
          idIcon.className = "card-icon active";
        } else if (state.door === "closed") {
          pd.className = "pill off";
          idIcon.className = "card-icon";
        } else {
          pd.className = "pill alert";
          idIcon.className = "card-icon danger";
        }

        const pg = document.getElementById("pill-gate");
        const ig = document.getElementById("icon-gate");
        pg.className = state.gate ? "pill on" : "pill off";
        pg.textContent = state.gate ? "OPEN" : "CLOSED";
        ig.className = state.gate ? "card-icon active" : "card-icon";
      }

      async function fetchState() {
        if (!isESP32Host) {
          showToast("Refreshed state (Demo Simulator)");
          return;
        }
        try {
          const [stRes, pkRes] = await Promise.all([
            fetch("/api/state")
              .then((r) => r.json())
              .catch(() => null),
            fetch("/api/parking")
              .then((r) => r.json())
              .catch(() => null),
          ]);
          if (stRes) {
            Object.assign(state, stRes);
            renderUI();
          }
          if (pkRes && pkRes.slots) {
            pkRes.slots.forEach((s, idx) => {
              const el =
                document.getElementById(`slot-${idx + 1}`) ||
                document.getElementById(s.id);
              if (el) {
                el.className = s.occupied
                  ? "slot-status occ"
                  : "slot-status free";
                el.textContent = s.occupied ? "OCCUPIED" : "FREE";
              }
            });
            const freeBadge = document.getElementById("free-slots-badge");
            if (freeBadge) freeBadge.textContent = `${pkRes.available} FREE`;
          }
        } catch (e) {}
      }

      // Auto-sync status every 2 seconds
      setInterval(fetchState, 2000);

      function toggleTheme() {
        const isDark = document.documentElement.classList.toggle("light");
        localStorage.setItem("theme", isDark ? "light" : "dark");
      }

      if (localStorage.getItem("theme") === "light") {
        document.documentElement.classList.add("light");
      }

      renderUI();
      fetchState();
    </script>
  </body>
</html>
)rawliteral";

#endif
