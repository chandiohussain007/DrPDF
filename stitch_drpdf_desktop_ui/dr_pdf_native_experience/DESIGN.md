---
name: Dr PDF Native Experience
colors:
  surface: '#0f131d'
  surface-dim: '#0f131d'
  surface-bright: '#353944'
  surface-container-lowest: '#0a0e18'
  surface-container-low: '#171b26'
  surface-container: '#1c1f2a'
  surface-container-high: '#262a35'
  surface-container-highest: '#313540'
  on-surface: '#dfe2f1'
  on-surface-variant: '#bbc9cf'
  inverse-surface: '#dfe2f1'
  inverse-on-surface: '#2c303b'
  outline: '#859399'
  outline-variant: '#3c494e'
  surface-tint: '#47d6ff'
  primary: '#a5e7ff'
  on-primary: '#003543'
  primary-container: '#00d2ff'
  on-primary-container: '#00566a'
  inverse-primary: '#00677f'
  secondary: '#89ceff'
  on-secondary: '#00344d'
  secondary-container: '#00a2e6'
  on-secondary-container: '#00344e'
  tertiary: '#ffd6a7'
  on-tertiary: '#472a00'
  tertiary-container: '#ffb148'
  on-tertiary-container: '#704500'
  error: '#ffb4ab'
  on-error: '#690005'
  error-container: '#93000a'
  on-error-container: '#ffdad6'
  primary-fixed: '#b6ebff'
  primary-fixed-dim: '#47d6ff'
  on-primary-fixed: '#001f28'
  on-primary-fixed-variant: '#004e60'
  secondary-fixed: '#c9e6ff'
  secondary-fixed-dim: '#89ceff'
  on-secondary-fixed: '#001e2f'
  on-secondary-fixed-variant: '#004c6e'
  tertiary-fixed: '#ffddb8'
  tertiary-fixed-dim: '#ffb95f'
  on-tertiary-fixed: '#2a1700'
  on-tertiary-fixed-variant: '#653e00'
  background: '#0f131d'
  on-background: '#dfe2f1'
  surface-variant: '#313540'
  surface-deep-midnight: '#06090f'
  surface-midnight-base: '#0b0f19'
  surface-midnight-elevated: '#111827'
  surface-midnight-overlay: '#1e293b'
  glass-panel-bg: rgba(17, 24, 39, 0.72)
  glass-pill-bg: rgba(30, 41, 59, 0.65)
  glass-border-hairline: rgba(255, 255, 255, 0.08)
  glass-border-highlight: rgba(255, 255, 255, 0.16)
  accent-electric-cyan: '#00d2ff'
  accent-azure-glow: '#38bdf8'
  accent-azure-solid: '#0ea5e9'
  security-gold-base: '#f59e0b'
  security-gold-light: '#fbbf24'
  security-gold-glow: rgba(245, 158, 11, 0.18)
  text-primary: '#f8fafc'
  text-secondary: '#94a3b8'
  text-muted: '#64748b'
  text-disabled: '#334155'
  system-red-danger: '#f43f5e'
  system-green-success: '#10b981'
typography:
  display-lg:
    fontFamily: Inter
    fontSize: 32px
    fontWeight: '600'
    lineHeight: 40px
    letterSpacing: -0.025em
  headline-lg:
    fontFamily: Inter
    fontSize: 22px
    fontWeight: '600'
    lineHeight: 28px
    letterSpacing: -0.015em
  headline-md:
    fontFamily: Inter
    fontSize: 18px
    fontWeight: '500'
    lineHeight: 24px
    letterSpacing: -0.01em
  title-sm:
    fontFamily: Inter
    fontSize: 14px
    fontWeight: '600'
    lineHeight: 20px
    letterSpacing: -0.005em
  body-md:
    fontFamily: Inter
    fontSize: 13px
    fontWeight: '400'
    lineHeight: 18px
    letterSpacing: 0em
  body-sm:
    fontFamily: Inter
    fontSize: 12px
    fontWeight: '400'
    lineHeight: 16px
    letterSpacing: 0.005em
  label-md:
    fontFamily: Inter
    fontSize: 11px
    fontWeight: '500'
    lineHeight: 14px
    letterSpacing: 0.02em
  code-sm:
    fontFamily: JetBrains Mono
    fontSize: 11px
    fontWeight: '500'
    lineHeight: 14px
    letterSpacing: -0.01em
  code-xs:
    fontFamily: JetBrains Mono
    fontSize: 10px
    fontWeight: '400'
    lineHeight: 12px
    letterSpacing: 0em
rounded:
  sm: 0.25rem
  DEFAULT: 0.5rem
  md: 0.75rem
  lg: 1rem
  xl: 1.5rem
  full: 9999px
spacing:
  space-2xs: 2px
  space-xs: 4px
  space-sm: 8px
  space-md: 12px
  space-lg: 16px
  space-xl: 20px
  space-2xl: 24px
  space-3xl: 32px
  titlebar-height: 44px
  sidebar-width-collapsed: 48px
  sidebar-width-expanded: 240px
  dock-floating-bottom: 24px
  panel-gutter: 12px
---

## Brand & Style

This design system translates the quiet confidence, high precision, and absolute privacy of a sovereign offline PDF environment into an Apple-inspired desktop visual architecture. Tailored for software engineers, legal counsels, researchers, and privacy-conscious professionals, the experience communicates instant response times, zero telemetry, and deep OS integration.

The aesthetic fuses modern **macOS Tahoe / Sequoia Glassmorphism** with **Technical Minimalism**:
- Translucent acrylic chrome layers (`backdrop-filter: blur(28px)`) that float above content rather than occluding it.
- Deep midnight canvas foundations contrasted with razor-sharp hairline borders (`0.5px - 1px` with subtle specular edge highlights).
- Vibrant electric cyan-azure accents rooted in the monogram logomark, accompanied by warm champagne-gold nodes reserved strictly for cryptographic trust, digital signatures, and security validations.
- Quiet, single-tier control bars and floating contextual pill palettes that replace cumbersome multi-tiered enterprise ribbons.

## Colors

The palette establishes an immersive, low-strain desktop darkroom tuned for precision document viewing and editing.

### Role Assignments
- **Primary (`#00d2ff`)**: Primary callouts, active viewport states, tool selection highlights, and micro-focus rings. Derived from the brightest edge of the monogram mark.
- **Secondary (`#0ea5e9`)**: Primary button surfaces, active tab indicators, and progress tracks.
- **Tertiary / Security Accent (`#f59e0b` / `#fbbf24`)**: Functional semantic color exclusively applied to digital signatures, PKCS#12 verification status, AES-256 encryption indicators, and high-priority redaction bounds.
- **Neutral Canvas (`#0b0f19`)**: Structural backdrop. Deepened to `#06090f` behind PDF rendering frames to provide crisp perceived contrast for rendered white documents.

### Functional Tinting & Glass Dynamics
Translucent panels utilize `#111827` at `72%` opacity paired with `backdrop-filter: blur(28px) saturate(180%)`. Border treatments avoid flat opaque colors; they use top-lit directional linear gradients (`rgba(255, 255, 255, 0.14)` falling off to `rgba(255, 255, 255, 0.03)`) simulating machined bevels.

## Typography

Typography prioritizes screen legibility at high pixel densities across macOS and Windows.

- **Primary Interface Font**: **Inter** (or native system San Francisco on macOS / Segoe UI Variable on Windows). Configured with optical metrics and negative tracking at title and headline scales to replicate native platform utility feel.
- **Monospaced Data Font**: **JetBrains Mono**. Applied to numerical dimensions (page scales, DPI, PDF bounding boxes, byte sizes), keystroke hotkeys (`⌘K`, `Ctrl+O`), and PKCS#12 hash representations.
- **Proportional Scaling**: Desktop tools rely on compact body heights (`12px` and `13px`) with optical vertical centering to maximize document viewing canvas while retaining strict readability.

## Layout & Spacing

The structural layout abandons deep legacy ribbons in favor of a three-zone **integrated desktop cockpit**:

1. **Unified Seamless Titlebar (44px)**: Blends OS window controls with active document metadata, breadcrumbs, unified primary action pills (Organize, Edit, Sign, Protect), and quick-action utility controls.
2. **Dynamic Work Canvas**: The document viewer occupies the central pane, bounded by optional collapsible inspector sidebars (`240px` expanded, collapsing into a sleek `48px` icon rail).
3. **Floating Contextual HUDs & Dock**: Annotation markers, page navigation, and zoom selectors sit in floating translucent pill docks hovering `24px` above the canvas bottom margin.

### Density & Rhythms
A strict 4px grid interval coordinates tool spacing. Interactive icon targets are minimum `28×28px` with `4px` internal padding, maintaining desktop-class cursor precision without unnecessary touch padding.

## Elevation & Depth

Visual depth is achieved through layered material translucency and edge-lit specular boundaries, avoiding heavy dropped shadows that muddy dark themes.

- **Level 0 (Document Void)**: `#06090f` flat background. The pure baseline where target document pages sit with a crisp 1px border (`rgba(255, 255, 255, 0.05)`) and a restrained outer shadow (`0 12px 36px rgba(0, 0, 0, 0.45)`).
- **Level 1 (Structural Chrome)**: `#0b0f19` at `80%` opacity with `backdrop-filter: blur(24px)`. Used on the top unified control bar and sidebars. Separated by micro-borders: `1px solid rgba(255, 255, 255, 0.07)`.
- **Level 2 (Floating Contextual Docks & Flyouts)**: `#1e293b` at `65%` opacity with `backdrop-filter: blur(32px) saturate(190%)`. Features a dual shadow: inner top specular highlight (`inset 0 1px 0 rgba(255, 255, 255, 0.15)`) and deep ambient blur (`0 16px 32px rgba(0, 0, 0, 0.35)`).
- **Level 3 (Modals & Command Palette `Ctrl+K`)**: Centered glass overlay bordered with `rgba(0, 210, 255, 0.25)` active outer ambient glow (`0 0 24px rgba(0, 210, 255, 0.12)`).

## Shapes

The interface balances soft industrial geometry with precise ergonomics:

- **Standard Elements (0.5rem / 8px)**: Dialog boxes, input fields, document thumbnails, tool properties panels, and dropdown menus.
- **Interactive Control Buttons & Segments (6px - 8px)**: Internal tool items within segmented groups use tight radii for modular cohesion.
- **Floating Controls & HUDs (9999px / Pill)**: Floating viewer controls (zoom, page counters, text annotation pills) use full capsule/pill curvature, reinforcing their floating, non-structural nature.

## Components

### Buttons & Interactive Controls
- **Primary CTA**: Electric Cyan gradient fill (`linear-gradient(135deg, #00d2ff 0%, #0ea5e9 100%)`), text `#06090f` (bold weight for contrast), subtle inner bevel highlight.
- **Secondary Ghost Pill**: Translucent surface (`rgba(255, 255, 255, 0.05)`), border `1px solid rgba(255, 255, 255, 0.1)`. Hover shifts background to `rgba(255, 255, 255, 0.1)`.
- **Security / Sign Action**: Champagne Gold background (`linear-gradient(135deg, #fbbf24 0%, #f59e0b 100%)`) with black text, triggered when signing or encrypting files.

### Unified Top Titlebar
- Single bar combining window handles, native title text, document dirty state indicator (`●`), tool segment switchers (View, Edit, Pages, Sign), and instant search (`⌘K`).
- Borderless look featuring single bottom boundary line: `1px solid rgba(255, 255, 255, 0.06)`.

### Floating Contextual Pill Bar
- Compact horizontal capsule containing page controls (`1 / 14`), zoom slider, fit-width/fit-page toggles, and selection marquee.
- Surface: Glass pill (`rgba(30, 41, 59, 0.7)`), inset white border `0.5px`, ambient backdrop blur `30px`.

### Collapsible Thumbnails & Bookmarks Sidebar
- **Collapsed**: 48px rail with vertical iconography (Thumbnails, Outline, Annotations, Signatures).
- **Expanded**: 240px dock showcasing live multi-page mini-renders with page sequence badges, drag-and-drop reorder handles, and deletion badges on hover.

### Inputs & Sliders
- Dark recessed input fields (`#06090f` with `1px solid rgba(255, 255, 255, 0.12)`). Focused state radiates an azure hairline glow (`#00d2ff` border with `0 0 0 2px rgba(0, 210, 255, 0.2)`).
- Zoom and compression sliders utilize ultra-slim `3px` tracks with `#00d2ff` thumb handles (`14px` circle with drop shadow).

### Modals & Command Palette (`Ctrl+K`)
- Centered floating glass sheet (`560px` max width), quick-filter search box, keyboard shortcut chips styled in monospaced format (`JetBrains Mono`, `rgba(255, 255, 255, 0.08)` surface, rounded 4px).