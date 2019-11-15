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

(defun color-theme-utils--underline-face-color (face)
  "Get underline color of FACE."
  (let ((underline (face-attribute face :underline nil t)))
    (when (listp underline)
      (alist-get :color underline nil))))

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

(defun color-theme-utils--colors ()
  `((EmacsDefaultBackground . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                     (face-background 'default nil t))
                                                                                    (list 2))))
    (EmacsDefaultForeground . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                     (face-foreground 'default nil t))
                                                                                    (list 2))))
    (EmacsHlLineBackground . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                    (or (face-background 'hl-line nil t) (face-background 'default  nil t)))
                                                                                   (list 2))))
    (EmacsHlLineForeground . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                    (or (face-foreground 'hl-line nil t) (face-foreground 'default  nil t)))
                                                                                   (list 2))))
    (EmacsModeLineBackground . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                      (or (face-background 'mode-line nil t) (face-background 'default  nil t)))
                                                                                     (list 2))))
    (EmacsModeLineForeground . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                      (or (face-foreground 'mode-line nil t) (face-foreground 'default nil t)))
                                                                                     (list 2))))
    (EmacsModeLineUnderline . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                     (or (color-theme-utils--underline-face-color 'mode-line) (face-background 'default nil t)))
                                                                                    (list 2))))
    (EmacsModeLineHighlightForeground . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                               (or (face-foreground 'mode-line-highlight nil t) (face-foreground 'default nil t)))
                                                                                              (list 2))))
    (EmacsModeLineInactiveBackground . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                              (or (face-background 'mode-line-inactive nil t) (face-background 'default  nil t)))
                                                                                             (list 2))))
    (EmacsModeLineInactiveForeground . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                              (or (face-foreground 'mode-line-inactive nil t) (face-foreground 'default nil t)))
                                                                                             (list 2))))
    (EmacsModeLineInactiveUnderline . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                             (or (color-theme-utils--underline-face-color 'mode-line-inactive) (face-background 'default nil t)))
                                                                                            (list 2))))
    (EmacsRegionBackground . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                    (or (face-background 'region nil t) (face-background 'default  nil t)))
                                                                                   (list 2))))
    (EmacsRegionForeground . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                    (or (face-foreground 'region nil t) (face-foreground 'default  nil t)))
                                                                                   (list 2))))
    (EmacsFringeBackground . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                    (or (face-background 'fringe nil t) (face-background 'default  nil t)))
                                                                                   (list 2))))
    (EmacsFringeForeground . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                    (or (face-foreground 'fringe nil t) (face-foreground 'default  nil t)))
                                                                                   (list 2))))
    (EmacsVerticalBorderBackground . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                            (or (face-background 'vertical-border nil t) (face-background 'default  nil t)))
                                                                                           (list 2))))
    (EmacsVerticalBorderForeground . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                            (or (face-foreground 'vertical-border nil t) (face-foreground 'default  nil t)))
                                                                                           (list 2))))
    (EmacsPopupMenuBackground . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                       (or (face-background 'popup-menu-face nil t) (face-background 'default  nil t)))
                                                                                      (list 2))))
    (EmacsPopupMenuForeground . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                       (or (face-foreground 'popup-menu-face nil t) (face-foreground 'default  nil t)))
                                                                                      (list 2))))
    (EmacsPopupMenuSelectionBackground . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                                (or (face-background 'popup-menu-selection-face nil t) (face-background 'default  nil t)))
                                                                                               (list 2))))
    (EmacsPopupMenuSelectionForeground . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                                (or (face-foreground 'popup-menu-selection-face nil t) (face-foreground 'default  nil t)))
                                                                                               (list 2))))
    (EmacsTooltipBackground . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                     (or (face-background 'tooltip nil t) (face-background 'default  nil t)))
                                                                                    (list 2))))
    (EmacsTooltipForeground . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                     (or (face-foreground 'tooltip nil t) (face-foreground 'default  nil t)))
                                                                                    (list 2))))
    (EmacsIsearchBackground . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                     (or (face-background 'isearch nil t) (face-background 'default  nil t)))
                                                                                    (list 2))))
    (EmacsIsearchForeground . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                     (or (face-foreground 'isearch nil t) (face-foreground 'default  nil t)))
                                                                                    (list 2))))
    (EmacsHighlightBackground . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                       (or (face-background 'highlight nil t) (face-background 'default nil t)))
                                                                                      (list 2))))
    (EmacsHighlightForeground . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                       (or (face-foreground 'highlight nil t) (face-foreground 'default nil t)))
                                                                                      (list 2))))
    (EmacsShadowForeground . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                    (or (face-foreground 'shadow nil t) (face-foreground 'default nil t)))
                                                                                   (list 2))))
    (EmacsCursorBackground . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                    (or (face-background 'cursor nil t) (face-background 'default nil t)))
                                                                                   (list 2))))
    (EmacsCursorForeground . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                    (or (face-foreground 'cursor nil t) (face-background 'default nil t)))
                                                                                   (list 2))))
    (EmacsKeywordForeground . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                     (or (face-foreground 'font-lock-keyword-face nil t) (face-foreground 'default nil t)))
                                                                                    (list 2))))
    (EmacsWarningForeground . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                     (or (face-foreground 'warning nil t) (face-foreground 'default nil t)))
                                                                                    (list 2))))
    (EmacsErrorForeground . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                   (or (face-foreground 'error nil t) (face-foreground 'default nil t)))
                                                                                  (list 2))))
    (EmacsSuccessForeground . ,(apply #'color-theme-utils--color-rgb-to-hex (append (color-theme-utils--color-name-to-rgb
                                                                                     (or (face-foreground 'success nil t) (face-foreground 'default nil t)))
                                                                                    (list 2))))))

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
  (message "Emacs color theme %s"
           (propertize (symbol-name (or (car custom-enabled-themes) 'default-theme)) 'face 'font-lock-variable-name-face))
  (when (color-theme-utils-export-file-save)
    (call-process-shell-command "i3-msg exec ~/bin/desktop-on-change")))

(defun color-theme-utils--export ()
  "Export current color theme definitions asynchronously."
  (run-with-idle-timer 0 nil #'color-theme-utils-export))

(add-hook 'after-enable-theme-hook 'color-theme-utils--export)

(provide 'color-theme-utils)

;;; color-theme-utils.el ends here
