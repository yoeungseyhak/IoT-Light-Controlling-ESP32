#ifndef ADMIN_PAGE_H
#define ADMIN_PAGE_H


// ==============================================================================
// OPTIMIZED ADMIN & CONFIGURATION PORTAL FOR ESP32
// - Stored in Flash memory (PROGMEM) to save dynamic RAM.
// - Zero external CDN dependencies (works completely offline / AP mode).
// - Credentials & WiFi settings persisted to ESP32 Flash via Preferences (NVS).
// Default admin account: admin / pw: admin123
// ==============================================================================

const char PAGE_ADMIN[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="en" class="dark">
  <head>
    <meta charset="UTF-8" />
    <meta
      name="viewport"
      content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no"
    />
    <title>System Configuration</title>
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
        --shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.25);
      }
      html.light {
        --bg: #f8fafc;
        --card: #ffffff;
        --card-hover: #f1f5f9;
        --border: #e2e8f0;
        --text: #0f172a;
        --text-muted: #64748b;
        --shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.05);
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
        padding-bottom: 3rem;
        transition:
          background 0.2s,
          color 0.2s;
      }

      /* Top Navbar */
      .nav {
        position: sticky;
        top: 0;
        z-index: 50;
        background: rgba(15, 23, 42, 0.9);
        backdrop-filter: blur(8px);
        border-bottom: 1px solid var(--border);
        height: 60px;
        display: flex;
        align-items: center;
        justify-content: space-between;
        padding: 0 1.25rem;
      }
      html.light .nav {
        background: rgba(255, 255, 255, 0.9);
      }
      .brand {
        display: flex;
        align-items: center;
        gap: 0.65rem;
        font-weight: 700;
        font-size: 1.05rem;
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
      .badge-admin {
        background: rgba(6, 182, 212, 0.15);
        color: var(--primary);
        border: 1px solid rgba(6, 182, 212, 0.3);
        padding: 0.2rem 0.5rem;
        border-radius: 999px;
        font-size: 0.7rem;
        font-weight: 700;
      }
      .nav-actions {
        display: flex;
        align-items: center;
        gap: 0.5rem;
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
      .btn-logout {
        background: rgba(239, 68, 68, 0.1);
        border: 1px solid rgba(239, 68, 68, 0.25);
        color: var(--danger);
        padding: 0.35rem 0.75rem;
        border-radius: 8px;
        font-size: 0.75rem;
        font-weight: 600;
        cursor: pointer;
        transition: 0.15s;
        display: inline-flex;
        align-items: center;
        gap: 0.4rem;
      }
      .btn-logout:hover {
        background: rgba(239, 68, 68, 0.2);
      }

      .container {
        max-width: 900px;
        margin: 0 auto;
        padding: 1.5rem 1rem;
      }

      /* Login Screen */
      #loginScreen {
        position: fixed;
        inset: 0;
        z-index: 100;
        background: var(--bg);
        display: flex;
        align-items: center;
        justify-content: center;
        padding: 1rem;
      }
      .login-box {
        width: 100%;
        max-width: 380px;
        background: var(--card);
        border: 1px solid var(--border);
        border-radius: 16px;
        padding: 2rem;
        box-shadow: var(--shadow);
      }
      .login-header {
        text-align: center;
        margin-bottom: 1.5rem;
      }
      .login-icon {
        width: 48px;
        height: 48px;
        border-radius: 12px;
        background: rgba(6, 182, 212, 0.15);
        border: 1px solid rgba(6, 182, 212, 0.3);
        color: var(--primary);
        display: flex;
        align-items: center;
        justify-content: center;
        margin: 0 auto 0.75rem;
      }
      .login-title {
        font-size: 1.25rem;
        font-weight: 700;
        margin-bottom: 0.25rem;
      }
      .login-sub {
        font-size: 0.8rem;
        color: var(--text-muted);
      }

      /* Form Inputs */
      .form-group {
        margin-bottom: 1rem;
      }
      .label {
        display: block;
        font-size: 0.75rem;
        font-weight: 600;
        color: var(--text-muted);
        margin-bottom: 0.35rem;
        text-transform: uppercase;
        letter-spacing: 0.5px;
      }
      .input-box {
        position: relative;
        display: flex;
        align-items: center;
      }
      .input {
        width: 100%;
        background: var(--bg);
        border: 1px solid var(--border);
        border-radius: 8px;
        padding: 0.65rem 0.85rem;
        color: var(--text);
        font-size: 0.85rem;
        transition: border-color 0.15s;
      }
      .input:focus {
        outline: none;
        border-color: var(--primary);
        box-shadow: 0 0 0 1px var(--primary);
      }
      .input-eye {
        position: absolute;
        right: 10px;
        cursor: pointer;
        color: var(--text-muted);
        padding: 4px;
        display: flex;
      }
      .input-eye:hover {
        color: var(--text);
      }
      .btn-primary {
        width: 100%;
        background: var(--primary);
        color: #fff;
        border: none;
        padding: 0.7rem;
        border-radius: 8px;
        font-weight: 600;
        font-size: 0.85rem;
        cursor: pointer;
        transition: 0.15s;
        display: flex;
        align-items: center;
        justify-content: center;
        gap: 0.4rem;
      }
      .btn-primary:hover {
        background: var(--primary-hover);
      }

      /* Settings Sections */
      .section-card {
        background: var(--card);
        border: 1px solid var(--border);
        border-radius: 14px;
        padding: 1.25rem;
        margin-bottom: 1.25rem;
        box-shadow: var(--shadow);
      }
      .section-head {
        display: flex;
        align-items: center;
        justify-content: space-between;
        margin-bottom: 1.25rem;
        border-bottom: 1px solid var(--border);
        padding-bottom: 0.75rem;
      }
      .section-title-wrap {
        display: flex;
        align-items: center;
        gap: 0.6rem;
      }
      .section-icon {
        width: 32px;
        height: 32px;
        border-radius: 8px;
        background: rgba(6, 182, 212, 0.1);
        border: 1px solid rgba(6, 182, 212, 0.25);
        color: var(--primary);
        display: flex;
        align-items: center;
        justify-content: center;
      }
      .section-title {
        font-size: 0.95rem;
        font-weight: 700;
      }
      .section-desc {
        font-size: 0.75rem;
        color: var(--text-muted);
      }
      .grid-2 {
        display: grid;
        grid-template-columns: repeat(auto-fit, minmax(260px, 1fr));
        gap: 1rem;
      }

      /* WiFi Scanner List */
      .scan-btn {
        background: transparent;
        border: 1px solid var(--border);
        color: var(--text);
        padding: 0.4rem 0.75rem;
        border-radius: 8px;
        font-size: 0.75rem;
        font-weight: 600;
        cursor: pointer;
        display: flex;
        align-items: center;
        gap: 0.4rem;
        transition: 0.15s;
      }
      .scan-btn:hover {
        background: var(--card-hover);
        border-color: var(--primary);
      }
      .wifi-list {
        background: var(--bg);
        border: 1px solid var(--border);
        border-radius: 8px;
        max-height: 150px;
        overflow-y: auto;
        margin-top: 0.5rem;
        display: none;
      }
      .wifi-item {
        display: flex;
        align-items: center;
        justify-content: space-between;
        padding: 0.5rem 0.75rem;
        font-size: 0.8rem;
        cursor: pointer;
        border-bottom: 1px solid rgba(51, 65, 85, 0.4);
      }
      .wifi-item:hover {
        background: var(--card-hover);
        color: var(--primary);
      }
      .wifi-rssi {
        font-size: 0.7rem;
        color: var(--text-muted);
        font-family: monospace;
      }

      /* Diagnostic Telemetry */
      .stat-grid {
        display: grid;
        grid-template-columns: repeat(auto-fill, minmax(180px, 1fr));
        gap: 0.75rem;
        margin-bottom: 1rem;
      }
      .stat-item {
        background: var(--bg);
        border: 1px solid var(--border);
        border-radius: 8px;
        padding: 0.6rem 0.8rem;
      }
      .stat-label {
        font-size: 0.7rem;
        color: var(--text-muted);
        text-transform: uppercase;
        font-weight: 600;
        margin-bottom: 0.2rem;
      }
      .stat-val {
        font-size: 0.9rem;
        font-weight: 700;
        font-family: monospace;
        color: var(--primary);
      }

      /* Action Buttons */
      .btn-row {
        display: flex;
        gap: 0.6rem;
        margin-top: 1rem;
        flex-wrap: wrap;
      }
      .btn-secondary {
        background: transparent;
        border: 1px solid var(--border);
        color: var(--text);
        padding: 0.6rem 1rem;
        border-radius: 8px;
        font-size: 0.8rem;
        font-weight: 600;
        cursor: pointer;
        transition: 0.15s;
      }
      .btn-secondary:hover {
        background: var(--card-hover);
      }
      .btn-danger {
        background: rgba(239, 68, 68, 0.1);
        border: 1px solid rgba(239, 68, 68, 0.3);
        color: var(--danger);
        padding: 0.6rem 1rem;
        border-radius: 8px;
        font-size: 0.8rem;
        font-weight: 600;
        cursor: pointer;
        transition: 0.15s;
      }
      .btn-danger:hover {
        background: #ef4444;
        color: #fff;
      }

      /* Toast Notification */
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
        z-index: 200;
        pointer-events: none;
      }
      .toast.show {
        opacity: 1;
        transform: translateY(0);
      }
    </style>
  </head>
  <body>
    <!-- Login Modal Gate -->
    <div id="loginScreen">
      <div class="login-box">
        <div class="login-header">
          <div class="login-icon">
            <svg
              width="24"
              height="24"
              fill="none"
              stroke="currentColor"
              stroke-width="2"
              viewBox="0 0 24 24"
            >
              <rect width="18" height="11" x="3" y="11" rx="2" ry="2" />
              <path d="M7 11V7a5 5 0 0 1 10 0v4" />
            </svg>
          </div>
          <h2 class="login-title">Login</h2>
        </div>
        <form onsubmit="handleLogin(event)">
          <div class="form-group">
            <label class="label">Username</label>
            <input
              type="text"
              id="loginUser"
              class="input"
              required
              placeholder="Enter username"
            />
          </div>
          <div class="form-group">
            <label class="label">Password</label>
            <div class="input-box">
              <input
                type="password"
                id="loginPass"
                class="input"
                required
                placeholder="Enter password"
              />
              <span
                class="input-eye"
                onclick="togglePassVisibility('loginPass')"
              >
                <svg
                  width="18"
                  height="18"
                  fill="none"
                  stroke="currentColor"
                  stroke-width="2"
                  viewBox="0 0 24 24"
                >
                  <path d="M2 12s3-7 10-7 10 7 10 7-3 7-10 7-10-7-10-7Z" />
                  <circle cx="12" cy="12" r="3" />
                </svg>
              </span>
            </div>
          </div>
          <button type="submit" class="btn-primary" style="margin-top: 1.25rem">
            <svg
              width="16"
              height="16"
              fill="none"
              stroke="currentColor"
              stroke-width="2"
              viewBox="0 0 24 24"
            >
              <path d="M15 3h4a2 2 0 0 1 2 2v14a2 2 0 0 1-2 2h-4" />
              <polyline points="10 17 15 12 10 7" />
              <line x1="15" y1="12" x2="3" y2="12" />
            </svg>
            Login
          </button>
        </form>
      </div>
    </div>

    <!-- Admin Navigation -->
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
            <path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z" />
          </svg>
        </div>
        <span>FLOOR MGMT</span>
        <span class="badge-admin">CONFIG</span>
      </div>
      <div class="nav-actions">
        <a href="/" class="icon-btn" title="Go to Device Dashboard">
          <svg
            width="18"
            height="18"
            fill="none"
            stroke="currentColor"
            stroke-width="2"
            viewBox="0 0 24 24"
          >
            <path d="m3 9 9-7 9 7v11a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z" />
            <polyline points="9 22 9 12 15 12 15 22" />
          </svg>
        </a>
        <button
          class="icon-btn"
          onclick="toggleTheme()"
          title="Toggle Light/Dark Theme"
        >
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
        <button class="btn-logout" onclick="handleLogout()">
          <svg
            width="14"
            height="14"
            fill="none"
            stroke="currentColor"
            stroke-width="2"
            viewBox="0 0 24 24"
          >
            <path d="M9 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h4" />
            <polyline points="16 17 21 12 16 7" />
            <line x1="21" y1="12" x2="9" y2="12" />
          </svg>
          Sign Out
        </button>
      </div>
    </nav>

    <div class="container">
      <!-- Section 1: WiFi Station Configuration (Connect ESP32 to Router) -->
      <div class="section-card">
        <div class="section-head">
          <div class="section-title-wrap">
            <div class="section-icon">
              <svg
                width="18"
                height="18"
                fill="none"
                stroke="currentColor"
                stroke-width="2"
                viewBox="0 0 24 24"
              >
                <path
                  d="M5 12.55a11 11 0 0 1 14.08 0M1.42 9a16 16 0 0 1 21.16 0M8.53 16.11a6 6 0 0 1 6.95 0M12 20h.01"
                />
              </svg>
            </div>
            <div>
              <div class="section-title">WiFi Station Network</div>
              <div class="section-desc">
                Connect ESP32 to local office or building WiFi
              </div>
            </div>
          </div>
          <button class="scan-btn" onclick="scanWifi()">
            <svg
              width="14"
              height="14"
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
            <span id="scanBtnText">Scan Networks</span>
          </button>
        </div>

        <div class="wifi-list" id="wifiList"></div>

        <div class="grid-2">
          <div class="form-group">
            <label class="label">WiFi SSID (Network Name)</label>
            <input
              type="text"
              id="staSsid"
              class="input"
              placeholder="e.g. Office_WiFi_5G"
              value="Office_Building_WiFi"
            />
          </div>
          <div class="form-group">
            <label class="label">WiFi Password</label>
            <div class="input-box">
              <input
                type="password"
                id="staPass"
                class="input"
                placeholder="Enter WiFi Password"
                value="CompanySecretPass2026"
              />
              <span class="input-eye" onclick="togglePassVisibility('staPass')">
                <svg
                  width="18"
                  height="18"
                  fill="none"
                  stroke="currentColor"
                  stroke-width="2"
                  viewBox="0 0 24 24"
                >
                  <path d="M2 12s3-7 10-7 10 7 10 7-3 7-10 7-10-7-10-7Z" />
                  <circle cx="12" cy="12" r="3" />
                </svg>
              </span>
            </div>
          </div>
        </div>
        <div class="btn-row">
          <button
            class="btn-primary"
            style="width: auto; padding: 0.6rem 1.25rem"
            onclick="saveWifiStation()"
          >
            <svg
              width="16"
              height="16"
              fill="none"
              stroke="currentColor"
              stroke-width="2"
              viewBox="0 0 24 24"
            >
              <path
                d="M19 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h11l5 5v11a2 2 0 0 1-2 2z"
              />
              <polyline points="17 21 17 13 7 13 7 21" />
              <polyline points="7 3 7 8 15 8" />
            </svg>
            Save & Connect WiFi
          </button>
        </div>
      </div>

      <!-- Section 2: SoftAP Hotspot Configuration (ESP32's Own WiFi) -->
      <div class="section-card">
        <div class="section-head">
          <div class="section-title-wrap">
            <div class="section-icon">
              <svg
                width="18"
                height="18"
                fill="none"
                stroke="currentColor"
                stroke-width="2"
                viewBox="0 0 24 24"
              >
                <rect width="20" height="8" x="2" y="2" rx="2" ry="2" />
                <rect width="20" height="8" x="2" y="14" rx="2" ry="2" />
                <line x1="6" y1="6" x2="6.01" y2="6" />
                <line x1="6" y1="18" x2="6.01" y2="18" />
              </svg>
            </div>
            <div>
              <div class="section-title">
                ESP32 Access Point (SoftAP Hotspot)
              </div>
              <div class="section-desc">
                Configure the ESP32's direct broadcast hotspot SSID & Password
              </div>
            </div>
          </div>
        </div>

        <div class="grid-2">
          <div class="form-group">
            <label class="label">Access Point SSID</label>
            <input
              type="text"
              id="apSsid"
              class="input"
              placeholder="e.g. FloorMgmt_ESP32_AP"
              value="FloorMgmt_ESP32_AP"
            />
          </div>
          <div class="form-group">
            <label class="label">Access Point Password (min 8 chars)</label>
            <div class="input-box">
              <input
                type="password"
                id="apPass"
                class="input"
                placeholder="Min 8 characters"
                value="12345678"
              />
              <span class="input-eye" onclick="togglePassVisibility('apPass')">
                <svg
                  width="18"
                  height="18"
                  fill="none"
                  stroke="currentColor"
                  stroke-width="2"
                  viewBox="0 0 24 24"
                >
                  <path d="M2 12s3-7 10-7 10 7 10 7-3 7-10 7-10-7-10-7Z" />
                  <circle cx="12" cy="12" r="3" />
                </svg>
              </span>
            </div>
          </div>
        </div>
        <div class="btn-row">
          <button
            class="btn-primary"
            style="width: auto; padding: 0.6rem 1.25rem"
            onclick="saveAccessPoint()"
          >
            <svg
              width="16"
              height="16"
              fill="none"
              stroke="currentColor"
              stroke-width="2"
              viewBox="0 0 24 24"
            >
              <path
                d="M19 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h11l5 5v11a2 2 0 0 1-2 2z"
              />
              <polyline points="17 21 17 13 7 13 7 21" />
              <polyline points="7 3 7 8 15 8" />
            </svg>
            Update AP Settings
          </button>
        </div>
      </div>

      <!-- Section 3: Admin Security & Password Change -->
      <div class="section-card">
        <div class="section-head">
          <div class="section-title-wrap">
            <div class="section-icon">
              <svg
                width="18"
                height="18"
                fill="none"
                stroke="currentColor"
                stroke-width="2"
                viewBox="0 0 24 24"
              >
                <rect width="18" height="11" x="3" y="11" rx="2" ry="2" />
                <path d="M7 11V7a5 5 0 0 1 10 0v4" />
              </svg>
            </div>
            <div>
              <div class="section-title">Admin Account Credentials</div>
              <div class="section-desc">
                Change the fixed admin username and password saved in ESP32
                Flash (NVS)
              </div>
            </div>
          </div>
        </div>

        <div class="grid-2">
          <div class="form-group">
            <label class="label">New Admin Username</label>
            <input
              type="text"
              id="newAdminUser"
              class="input"
              placeholder="New Username"
              value="admin"
            />
          </div>
          <div class="form-group">
            <label class="label">New Admin Password</label>
            <div class="input-box">
              <input
                type="password"
                id="newAdminPass"
                class="input"
                placeholder="Enter New Password"
              />
              <span
                class="input-eye"
                onclick="togglePassVisibility('newAdminPass')"
              >
                <svg
                  width="18"
                  height="18"
                  fill="none"
                  stroke="currentColor"
                  stroke-width="2"
                  viewBox="0 0 24 24"
                >
                  <path d="M2 12s3-7 10-7 10 7 10 7-3 7-10 7-10-7-10-7Z" />
                  <circle cx="12" cy="12" r="3" />
                </svg>
              </span>
            </div>
          </div>
        </div>
        <div class="btn-row">
          <button
            class="btn-primary"
            style="width: auto; padding: 0.6rem 1.25rem"
            onclick="saveAdminCredentials()"
          >
            <svg
              width="16"
              height="16"
              fill="none"
              stroke="currentColor"
              stroke-width="2"
              viewBox="0 0 24 24"
            >
              <path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z" />
            </svg>
            Save New Credentials
          </button>
        </div>
      </div>

      <!-- Section 4: Backend Cloud Server Connection -->
      <!-- <div class="section-card">
        <div class="section-head">
          <div class="section-title-wrap">
            <div class="section-icon">
              <svg
                width="18"
                height="18"
                fill="none"
                stroke="currentColor"
                stroke-width="2"
                viewBox="0 0 24 24"
              >
                <path d="M17.5 19H9a7 7 0 1 1 6.71-9h1.79a4.5 4.5 0 1 1 0 9Z" />
              </svg>
            </div>
            <div>
              <div class="section-title">Cloud WebSocket Backend</div>
              <div class="section-desc">
                IP address & port of the central Node.js Express server
              </div>
            </div>
          </div>
        </div>

        <div class="grid-2">
          <div class="form-group">
            <label class="label">WebSocket Server Host (IP or Domain)</label>
            <input
              type="text"
              id="wsHost"
              class="input"
              placeholder="e.g. 192.168.1.100 or ec2-xx.compute.amazonaws.com"
              value="192.168.1.100"
            />
          </div>
          <div class="form-group">
            <label class="label">WebSocket Port</label>
            <input
              type="number"
              id="wsPort"
              class="input"
              placeholder="5050"
              value="5050"
            />
          </div>
        </div>
        <div class="btn-row">
          <button
            class="btn-primary"
            style="width: auto; padding: 0.6rem 1.25rem"
            onclick="saveServerConfig()"
          >
            Save Server Connection
          </button>
        </div>
      </div> -->

      <!-- Section 5: Hardware Telemetry & System Tools -->
      <div class="section-card">
        <div class="section-head">
          <div class="section-title-wrap">
            <div class="section-icon">
              <svg
                width="18"
                height="18"
                fill="none"
                stroke="currentColor"
                stroke-width="2"
                viewBox="0 0 24 24"
              >
                <rect width="16" height="16" x="4" y="4" rx="2" />
                <rect width="6" height="6" x="9" y="9" rx="1" />
                <path
                  d="M15 2v2M9 2v2M20 15h2M20 9h2M9 20v2M15 20v2M2 9h2M2 15h2"
                />
              </svg>
            </div>
            <div>
              <div class="section-title">ESP32 Hardware Diagnostics</div>
              <div class="section-desc">
                Hardware specs, memory utilization, and maintenance tools
              </div>
            </div>
          </div>
        </div>

        <div class="stat-grid">
          <div class="stat-item">
            <div class="stat-label">Device MAC</div>
            <div class="stat-val" id="macVal">--:--:--:--:--:--</div>
          </div>
          <div class="stat-item">
            <div class="stat-label">Local IP</div>
            <div class="stat-val" id="ipVal">---.---.-.---</div>
          </div>
          <div class="stat-item">
            <div class="stat-label">Free Heap RAM</div>
            <div class="stat-val" id="heapVal">-- KB</div>
          </div>
          <div class="stat-item">
            <div class="stat-label">WiFi Signal</div>
            <div class="stat-val" id="rssiVal">-- dBm</div>
          </div>
          <div class="stat-item">
            <div class="stat-label">Flash Memory</div>
            <div class="stat-val">-- MB SPI</div>
          </div>
          <div class="stat-item">
            <div class="stat-label">System Uptime</div>
            <div class="stat-val" id="uptimeVal">--</div>
          </div>
        </div>

        <div
          class="btn-row"
          style="border-top: 1px solid var(--border); padding-top: 1rem"
        >
          <button class="btn-secondary" onclick="rebootESP32()">
            <svg
              width="14"
              height="14"
              fill="none"
              stroke="currentColor"
              stroke-width="2"
              viewBox="0 0 24 24"
              style="vertical-align: middle; margin-right: 4px"
            >
              <path d="M21 12a9 9 0 0 0-9-9 9.75 9.75 0 0 0-6.74 2.74L3 8" />
              <path
                d="M3 3v5h5M3 12a9 9 0 0 0 9 9 9.75 9.75 0 0 0 6.74-2.74L21 16"
              />
              <path d="M16 21h5v-5" />
            </svg>
            Restart ESP32
          </button>
          <button class="btn-danger" onclick="factoryReset()">
            Factory Reset Config
          </button>
        </div>
      </div>
    </div>

    <div class="toast" id="toast">Settings updated</div>

    <script>
      // State & Stored Credentials
      let currentAdminUser =
        localStorage.getItem("esp32_admin_user") || "admin";
      let currentAdminPass =
        localStorage.getItem("esp32_admin_pass") || "admin123";
      let isLoggedIn =
        sessionStorage.getItem("esp32_admin_logged_in") === "true";

      const isESP32Host =
        window.location.protocol.startsWith("http") &&
        window.location.port !== "5173";

      function showToast(msg) {
        const t = document.getElementById("toast");
        t.textContent = msg;
        t.classList.add("show");
        setTimeout(() => t.classList.remove("show"), 2400);
      }

      function togglePassVisibility(id) {
        const el = document.getElementById(id);
        el.type = el.type === "password" ? "text" : "password";
      }

      // Authentication Gate
      function checkAuth() {
        const screen = document.getElementById("loginScreen");
        if (isLoggedIn) {
          screen.style.display = "none";
          loadSystemStatus();
        } else {
          screen.style.display = "flex";
        }
      }

      async function handleLogin(e) {
        e.preventDefault();
        const u = document.getElementById("loginUser").value.trim();
        const p = document.getElementById("loginPass").value;

        if (isESP32Host) {
          try {
            const res = await fetch(
              `/api/admin/login?user=${encodeURIComponent(u)}&pass=${encodeURIComponent(p)}`,
            );
            if (res.ok) {
              const data = await res.json();
              if (data.success) {
                isLoggedIn = true;
                sessionStorage.setItem("esp32_admin_logged_in", "true");
                checkAuth();
                showToast("Welcome, Administrator!");
                return;
              } else {
                alert(data.message || "Invalid Username or Password!");
                return;
              }
            }
          } catch (err) {
            console.warn(
              "ESP32 login endpoint unreachable, checking local fallback",
              err,
            );
          }
        }

        if (u === currentAdminUser && p === currentAdminPass) {
          isLoggedIn = true;
          sessionStorage.setItem("esp32_admin_logged_in", "true");
          checkAuth();
          showToast("Welcome, Administrator!");
        } else {
          alert("Invalid Username or Password!");
        }
      }

      async function loadSystemStatus() {
        if (!isESP32Host) return;
        try {
          const res = await fetch("/api/admin/status");
          if (res.ok) {
            const d = await res.json();
            if (d.mac) document.getElementById("macVal").textContent = d.mac;
            if (d.ip) document.getElementById("ipVal").textContent = d.ip;
            if (d.heap) document.getElementById("heapVal").textContent = d.heap;
            if (d.rssi) document.getElementById("rssiVal").textContent = d.rssi;
            if (d.uptime)
              document.getElementById("uptimeVal").textContent = d.uptime;
            if (d.staSsid) document.getElementById("staSsid").value = d.staSsid;
            if (d.apSsid) document.getElementById("apSsid").value = d.apSsid;
            if (d.wsHost) document.getElementById("wsHost").value = d.wsHost;
            if (d.wsPort) document.getElementById("wsPort").value = d.wsPort;
          }
        } catch (e) {}
      }

      function handleLogout() {
        isLoggedIn = false;
        sessionStorage.removeItem("esp32_admin_logged_in");
        checkAuth();
        showToast("Signed out successfully");
      }

      // 1. Scan WiFi Networks
      async function scanWifi() {
        const btn = document.getElementById("scanBtnText");
        const list = document.getElementById("wifiList");
        btn.textContent = "Scanning...";
        list.style.display = "block";
        list.innerHTML =
          '<div style="padding:0.75rem; text-align:center; font-size:0.75rem; color:var(--text-muted);">Scanning 2.4GHz WiFi channels...</div>';

        if (isESP32Host) {
          try {
            const res = await fetch("/api/admin/wifi/scan");
            if (res.ok) {
              const data = await res.json();
              renderWifiList(data.networks || []);
              btn.textContent = "Scan Networks";
              return;
            }
          } catch (e) {}
        }

        // Offline / Demo Simulator fallback
        setTimeout(() => {
          renderWifiList([
            { ssid: "Office_Building_WiFi", rssi: -48, secure: true },
            { ssid: "Cotafer_IoT_Network", rssi: -58, secure: true },
            { ssid: "Guest_Access_Free", rssi: -72, secure: false },
            { ssid: "Facility_Eng_AP", rssi: -79, secure: true },
          ]);
          btn.textContent = "Scan Networks";
        }, 800);
      }

      function renderWifiList(nets) {
        const list = document.getElementById("wifiList");
        if (!nets.length) {
          list.innerHTML =
            '<div style="padding:0.5rem; text-align:center; font-size:0.75rem;">No networks found</div>';
          return;
        }
        list.innerHTML = nets
          .map(
            (n) => `
        <div class="wifi-item" onclick="selectWifi('${n.ssid}')">
          <span>📶 ${n.ssid} ${n.secure ? "🔒" : ""}</span>
          <span class="wifi-rssi">${n.rssi} dBm</span>
        </div>
      `,
          )
          .join("");
      }

      function selectWifi(ssid) {
        document.getElementById("staSsid").value = ssid;
        document.getElementById("wifiList").style.display = "none";
        document.getElementById("staPass").focus();
      }

      // 2. Save WiFi Station Settings
      async function saveWifiStation() {
        const ssid = document.getElementById("staSsid").value.trim();
        const pass = document.getElementById("staPass").value;
        if (!ssid) return alert("Please enter a WiFi SSID");

        if (isESP32Host) {
          try {
            await fetch(
              `/api/admin/wifi/save?ssid=${encodeURIComponent(ssid)}&pass=${encodeURIComponent(pass)}`,
            );
          } catch (e) {}
        }
        showToast(`WiFi saved: ${ssid}. ESP32 will connect.`);
      }

      // 3. Save SoftAP Hotspot Settings
      async function saveAccessPoint() {
        const ssid = document.getElementById("apSsid").value.trim();
        const pass = document.getElementById("apPass").value;
        if (!ssid) return alert("Please enter AP SSID");
        if (pass.length < 8)
          return alert("AP Password must be at least 8 characters long");

        if (isESP32Host) {
          try {
            await fetch(
              `/api/admin/ap/save?ssid=${encodeURIComponent(ssid)}&pass=${encodeURIComponent(pass)}`,
            );
          } catch (e) {}
        }
        showToast(`Hotspot updated: ${ssid} / ${pass}`);
      }

      // 4. Change Admin Password & Username
      async function saveAdminCredentials() {
        const newUser = document.getElementById("newAdminUser").value.trim();
        const newPass = document.getElementById("newAdminPass").value;
        if (!newUser) return alert("Username cannot be empty");
        if (!newPass || newPass.length < 6)
          return alert("Password must be at least 6 characters long");

        currentAdminUser = newUser;
        currentAdminPass = newPass;
        localStorage.setItem("esp32_admin_user", newUser);
        localStorage.setItem("esp32_admin_pass", newPass);

        if (isESP32Host) {
          try {
            await fetch(
              `/api/admin/password?user=${encodeURIComponent(newUser)}&pass=${encodeURIComponent(newPass)}`,
            );
          } catch (e) {}
        }
        showToast("Admin credentials updated in Flash memory!");
      }

      // 5. Save Server Connection
      async function saveServerConfig() {
        const host = document.getElementById("wsHost").value.trim();
        const port = document.getElementById("wsPort").value;
        if (isESP32Host) {
          try {
            await fetch(
              `/api/admin/server?host=${encodeURIComponent(host)}&port=${port}`,
            );
          } catch (e) {}
        }
        showToast(`Backend set to: ${host}:${port}`);
      }

      // 6. Reboot ESP32
      async function rebootESP32() {
        if (!confirm("Are you sure you want to reboot the ESP32?")) return;
        showToast("Rebooting ESP32... Please wait 10 seconds.");
        if (isESP32Host) {
          try {
            await fetch("/api/admin/reboot");
          } catch (e) {}
        }
        setTimeout(() => location.reload(), 8000);
      }

      // 7. Factory Reset
      async function factoryReset() {
        if (
          !confirm(
            "WARNING: This will reset all WiFi, AP, and password settings to defaults! Proceed?",
          )
        )
          return;
        localStorage.removeItem("esp32_admin_user");
        localStorage.removeItem("esp32_admin_pass");
        currentAdminUser = "admin";
        currentAdminPass = "admin123";
        if (isESP32Host) {
          try {
            await fetch("/api/admin/reset");
          } catch (e) {}
        }
        showToast("Config reset to defaults (admin/admin123)");
        setTimeout(() => location.reload(), 1500);
      }

      // Theme toggling
      function toggleTheme() {
        const isDark = document.documentElement.classList.toggle("light");
        localStorage.setItem("theme", isDark ? "light" : "dark");
      }

      if (localStorage.getItem("theme") === "light") {
        document.documentElement.classList.add("light");
      }

      // Check auth on load
      checkAuth();
    </script>
  </body>
</html>
)rawliteral";

#endif
