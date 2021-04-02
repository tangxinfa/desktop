;;; color-theme-utils.el --- Color theme utils  -*- lexical-binding: t; -*-

;; Copyright (C) 2018  tangxinfa

;; Author: tangxinfa <tangxinfa@gmail.com>
;; Keywords: tools

;; This program is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation, either version 3 of the License, or
;; (at your option) any later version.

;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with this program.  If not, see <https://www.gnu.org/licenses/>.

;;; Commentary:

;; Export color theme definitions for other applications to use.

;;; Code:
(require 'color)
(require 'color-theme-approximate)

;; Support hook before/after theme enable and disable.
(defvar before-enable-theme-hook nil
  "Hook run before a color theme is enabled using `enable-theme'.")
(defvar after-enable-theme-hook nil
  "Hook run after a color theme is enabled using `enable-theme'.")
(defun ad-enable-theme-run-hooks (orig-fun theme)
  "Run `before-enable-theme-hook' and `after-enable-theme-hook' before/after enable THEME."
  (let ((result nil))
    (unless (eq theme 'user)
      (run-hooks 'before-enable-theme-hook))
    (setq result (funcall orig-fun theme))
    (unless (eq theme 'user)
      (run-hooks 'after-enable-theme-hook))
    result))
(advice-add 'enable-theme :around #'ad-enable-theme-run-hooks)

(defvar before-disable-theme-hook nil
  "Hook run before a color theme is disabled using `disable-theme'.")
(defvar after-disable-theme-hook nil
  "Hook run after a color theme is disabled using `disable-theme'.")
(defun ad-disable-theme-run-hooks (orig-fun theme)
  "Run `before-disable-theme-hook' `after-disable-theme-hook' before/after disable THEME."
  (let ((result nil)
        (valid (custom-theme-enabled-p theme)))
    (when valid
      (run-hooks 'before-disable-theme-hook))
    (setq result (funcall orig-fun theme))
    (when valid
      (run-hooks 'after-disable-theme-hook))
    result))
(advice-add 'disable-theme :around #'ad-disable-theme-run-hooks)

;; Disable any enabled themes before enabling a new theme.
(defun ad-enable-theme-disable-all-enabled-themes (theme)
  "Disable any enabled themes before enabling a new THEME.
This prevents overlapping themes; something I would rarely want."
  (unless (eq theme 'user)
    (mapcar #'disable-theme custom-enabled-themes)))
(advice-add 'enable-theme :before #'ad-enable-theme-disable-all-enabled-themes)

;; Export color theme definitions into files.
(defvar color-theme-utils-export-file "~/.config/emacs.theme"
  "YAML front matter file to save Emacs color theme definitions.")

(defun color-theme-utils--face-underline-color (face)
  "Get underline color of FACE."
  (let* ((underline (face-attribute face :underline nil t))
         (color (if (stringp underline)
                    underline
                  (when (listp underline)
                    (alist-get :color underline nil)))))
    (if (stringp color)
        color)))

(defun color-theme-utils--face-overline-color (face)
  "Get overline color of FACE."
  (let ((overline (face-attribute face :overline nil t)))
    (if (stringp overline)
        overline)))

(defun color-theme-utils--color-name-to-rgb (color)
  "A `color-name-to-rgb' wrap for process special colors."
  (cond ((string-equal color "unspecified-fg")
         (if window-system '(0 0 0) '(1 1 1)))
        ((equal color "unspecified-bg")
         (if window-system '(1 1 1) '(0 0 0)))
        (t (color-name-to-rgb color))))

(defun color-theme-utils--color-rgb-to-hex (red green blue &optional digits-per-component)
  "Like `color-rgb-to-hex' but without # prefix."
  (string-trim-left (color-rgb-to-hex red green blue digits-per-component) "#"))

(defun color-theme-utils--face-invisible (face)
  (let ((height (face-attribute face :height nil t)))
    (cond
     ((numberp height) (<= height 1))
     (t nil))))

(defun color-theme-utils--face-background (face &optional force fallback)
  "Get background color of FACE."
  (let ((color
         (apply #'color-theme-utils--color-rgb-to-hex
                (append (color-theme-utils--color-name-to-rgb
                         (or (face-background face nil t)
                             (face-background (or fallback 'default) nil t)
                             "#FFFFFF"))
                        (list 2)))))
    (unless force
      (when (or (color-theme-utils--face-invisible face) (equal color (color-theme-utils--face-foreground face t)))
        (setq color (color-theme-utils--face-background (or fallback 'default)))))
    color))

(defun color-theme-utils--face-foreground (face &optional force fallback)
  "Get foreground color of FACE."
  (let ((color
         (apply #'color-theme-utils--color-rgb-to-hex
                (append (color-theme-utils--color-name-to-rgb
                         (or (face-foreground face nil t)
                             (face-foreground (or fallback 'default) nil t)
                             "#000000"))
                        (list 2)))))
    (unless force
      (when (or (color-theme-utils--face-invisible face) (equal color (color-theme-utils--face-background face t)))
        (setq color (color-theme-utils--face-foreground (or fallback 'default)))))
    color))

(defun color-theme-utils--face-underline (face &optional force fallback)
  "Get underline color of FACE."
  (let ((color
         (apply #'color-theme-utils--color-rgb-to-hex
                (append (color-theme-utils--color-name-to-rgb
                         (or (color-theme-utils--face-underline-color face)
                             (face-background 'default nil t)
                             "#FFFFFF"))
                        (list 2)))))
    (unless force
      (when (or (color-theme-utils--face-invisible face) (equal (color-theme-utils--face-foreground face t)  (color-theme-utils--face-background face t)))
        (setq color (color-theme-utils--face-underline (or fallback 'default)))))
    color))

(defun color-theme-utils--face-overline (face &optional force fallback)
  "Get overline color of FACE."
  (let ((color
         (apply #'color-theme-utils--color-rgb-to-hex
                (append (color-theme-utils--color-name-to-rgb
                         (or (color-theme-utils--face-overline-color face)
                             (face-background 'default nil t)
                             "#FFFFFF"))
                        (list 2)))))
    (unless force
      (when (or (color-theme-utils--face-invisible face) (equal (color-theme-utils--face-foreground face t)  (color-theme-utils--face-background face t)))
        (setq color (color-theme-utils--face-overline (or fallback 'default)))))
    color))

(defun color-theme-utils--colors ()
  `((EmacsDefaultBackground . ,(color-theme-utils--face-background 'default))
    (EmacsDefaultForeground . ,(color-theme-utils--face-foreground 'default))
    (EmacsHlLineBackground . ,(color-theme-utils--face-background 'hl-line))
    (EmacsHlLineForeground . ,(color-theme-utils--face-foreground 'hl-line))
    (EmacsModeLineBackground . ,(color-theme-utils--face-background 'mode-line nil 'default))
    (EmacsModeLineForeground . ,(color-theme-utils--face-foreground 'mode-line nil 'default))
    (EmacsModeLineUnderline . ,(color-theme-utils--face-underline 'mode-line nil 'default))
    (EmacsModeLineHighlightForeground . ,(color-theme-utils--face-foreground 'mode-line-highlight nil 'font-lock-keyword-face))
    (EmacsModeLineInactiveBackground . ,(color-theme-utils--face-background 'mode-line-inactive nil 'shadow))
    (EmacsModeLineInactiveForeground . ,(color-theme-utils--face-foreground 'mode-line-inactive nil 'shadow))
    (EmacsModeLineInactiveUnderline . ,(color-theme-utils--face-underline 'mode-line-inactive nil 'shadow))
    (EmacsRegionBackground . ,(color-theme-utils--face-background 'region))
    (EmacsRegionForeground . ,(color-theme-utils--face-foreground 'region))
    (EmacsFringeBackground . ,(color-theme-utils--face-background 'fringe))
    (EmacsFringeForeground . ,(color-theme-utils--face-foreground 'fringe))
    (EmacsVerticalBorderBackground . ,(color-theme-utils--face-background 'vertical-border))
    (EmacsVerticalBorderForeground . ,(color-theme-utils--face-foreground 'vertical-border))
    (EmacsPopupMenuBackground . ,(color-theme-utils--face-background 'popup-menu-face))
    (EmacsPopupMenuForeground . ,(color-theme-utils--face-foreground 'popup-menu-face))
    (EmacsPopupMenuSelectionBackground . ,(color-theme-utils--face-background 'popup-menu-selection-face))
    (EmacsPopupMenuSelectionForeground . ,(color-theme-utils--face-foreground 'popup-menu-selection-face))
    (EmacsTooltipBackground . ,(color-theme-utils--face-background 'tooltip))
    (EmacsTooltipForeground . ,(color-theme-utils--face-foreground 'tooltip))
    (EmacsIsearchBackground . ,(color-theme-utils--face-background 'isearch))
    (EmacsIsearchForeground . ,(color-theme-utils--face-foreground 'isearch))
    (EmacsHighlightBackground . ,(color-theme-utils--face-background 'highlight))
    (EmacsHighlightForeground . ,(color-theme-utils--face-foreground 'highlight))
    (EmacsShadowForeground . ,(color-theme-utils--face-foreground 'shadow))
    (EmacsCursorBackground . ,(color-theme-utils--face-background 'cursor))
    (EmacsCursorForeground . ,(color-theme-utils--face-foreground 'cursor))
    (EmacsKeywordForeground . ,(color-theme-utils--face-foreground 'font-lock-keyword-face))
    (EmacsWarningForeground . ,(color-theme-utils--face-foreground 'warning))
    (EmacsErrorForeground . ,(color-theme-utils--face-foreground 'error))
    (EmacsSuccessForeground . ,(color-theme-utils--face-foreground 'success))))

(defun color-theme-utils-basic-color-name (color)
  "Basic color name of COLOR."
  (let* ((basic-color-names '("black" "red" "green" "yellow" "blue" "magenta" "cyan" "white"))
         (ca-defined-rgb-list (mapcar #'ca-color-to-rgb basic-color-names))
         (color-value (ca-approximate (ca-color-to-rgb color)))
         (color-index (cl-position color-value ca-defined-rgb-list)))
    (nth color-index basic-color-names)))

(defun color-theme-utils-export-file-save ()
  "Export Emacs color theme to file."
  (let ((old-contents "")
        (colors (color-theme-utils--colors)))
    (with-temp-buffer
      (ignore-errors
        (insert-file-contents color-theme-utils-export-file)
        (setq old-contents (buffer-string)))
      (erase-buffer)
      (insert "---\n")
      (dolist (color colors)
        (insert (format "%s: \"%s\"\n" (car color) (cdr color))))
      (dolist (color colors)
        (insert (format "Basic%s: \"%s\"\n" (car color) (color-theme-utils-basic-color-name (concat "#" (cdr color))))))
      (insert "---\n")
      (unless (string= old-contents (buffer-string))
        (let (message-log-max)
          (write-region (point-min) (point-max) color-theme-utils-export-file)
          t)))))

(defun color-theme-utils-export ()
  "Export currrent color theme definitions."
  (interactive)
  (when (color-theme-utils-export-file-save)
    (call-process-shell-command "i3-msg exec ~/bin/desktop-on-change"))
  (message "Emacs color theme %s"
           (propertize (symbol-name (or (car custom-enabled-themes) 'default-theme)) 'face 'font-lock-variable-name-face)))

(defun color-theme-utils--export ()
  "Export current color theme definitions asynchronously."
  (run-with-idle-timer 0 nil #'color-theme-utils-export))

(add-hook 'after-enable-theme-hook 'color-theme-utils--export)

(provide 'color-theme-utils)

;;; color-theme-utils.el ends here
