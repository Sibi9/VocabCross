using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;

namespace Antigravity.GameUI
{
    /// <summary>
    /// Production-ready Saga Map Controller implementing the game-ui-ux skill patterns:
    /// - Safe Area Inset adaptation for notched mobile screens.
    /// - Event-driven level progression (NO per-frame polling).
    /// - Keyboard / Gamepad focus navigation support.
    /// - Smooth scrolling to the active level node.
    /// </summary>
    public class SagaMapController : MonoBehaviour
    {
        [Header("UI References")]
        [SerializeField] private RectTransform safeAreaPanel;
        [SerializeField] private ScrollRect mapScrollRect;
        [SerializeField] private RectTransform mapContent;
        [SerializeField] private Button[] levelNodeButtons;
        
        [Header("Sprites Configuration")]
        [SerializeField] private Sprite activeNodeSprite;
        [SerializeField] private Sprite stoneNodeSprite;
        [SerializeField] private Sprite goldNodeSprite;
        [SerializeField] private Sprite goldStarSprite;
        [SerializeField] private Sprite emptyStarSprite;
        
        [Header("Game State")]
        [SerializeField] private int currentUnlockedLevel = 1;

        // Events decoupled from UI rendering
        public static event Action<int> OnLevelSelected;
        public static event Action<int, int> OnLevelCompleted; // levelId, starsEarned

        private void Awake()
        {
            ApplySafeArea();
        }

        private void Start()
        {
            InitializeMapNodes();
            SetupFocusNavigation();
            ScrollToNode(currentUnlockedLevel);
        }

        private void OnEnable()
        {
            OnLevelCompleted += HandleLevelCompleted;
        }

        private void OnDisable()
        {
            OnLevelCompleted -= HandleLevelCompleted;
        }

        /// <summary>
        /// Adapts panel anchors to OS Safe Area (notches, curved corners, home bar)
        /// </summary>
        private void ApplySafeArea()
        {
            if (safeAreaPanel == null) return;

            Rect safeArea = Screen.safeArea;
            Vector2 minAnchor = safeArea.position;
            Vector2 maxAnchor = minAnchor + safeArea.size;

            minAnchor.x /= Screen.width;
            minAnchor.y /= Screen.height;
            maxAnchor.x /= Screen.width;
            maxAnchor.y /= Screen.height;

            safeAreaPanel.anchorMin = minAnchor;
            safeAreaPanel.anchorMax = maxAnchor;
        }

        /// <summary>
        /// Configures visual states and click handlers for each level node
        /// </summary>
        private void InitializeMapNodes()
        {
            for (int i = 0; i < levelNodeButtons.Length; i++)
            {
                int levelIndex = i + 1;
                Button btn = levelNodeButtons[i];
                if (btn == null) continue;

                Image btnImage = btn.GetComponent<Image>();
                bool isUnlocked = levelIndex <= currentUnlockedLevel;
                bool isActive = levelIndex == currentUnlockedLevel;

                btn.interactable = isUnlocked;

                if (btnImage != null)
                {
                    if (isActive)
                        btnImage.sprite = activeNodeSprite;
                    else if (isUnlocked)
                        btnImage.sprite = goldNodeSprite;
                    else
                        btnImage.sprite = stoneNodeSprite;
                }

                btn.onClick.RemoveAllListeners();
                btn.onClick.AddListener(() => SelectLevel(levelIndex));
            }
        }

        /// <summary>
        /// Configures explicit navigation neighbors for Gamepad / Keyboard D-Pad
        /// </summary>
        private void SetupFocusNavigation()
        {
            for (int i = 0; i < levelNodeButtons.Length; i++)
            {
                Navigation nav = new Navigation { mode = Navigation.Mode.Explicit };
                if (i > 0)
                    nav.selectOnDown = levelNodeButtons[i - 1];
                if (i < levelNodeButtons.Length - 1)
                    nav.selectOnUp = levelNodeButtons[i + 1];

                levelNodeButtons[i].navigation = nav;
            }

            // Set initial focus to the active level node
            if (currentUnlockedLevel > 0 && currentUnlockedLevel <= levelNodeButtons.Length)
            {
                EventSystem.current?.SetSelectedGameObject(levelNodeButtons[currentUnlockedLevel - 1].gameObject);
            }
        }

        public void SelectLevel(int levelId)
        {
            Debug.Log($"[SagaMap] Level {levelId} selected.");
            OnLevelSelected?.Invoke(levelId);
        }

        private void HandleLevelCompleted(int levelId, int stars)
        {
            if (levelId >= currentUnlockedLevel && currentUnlockedLevel < levelNodeButtons.Length)
            {
                currentUnlockedLevel = levelId + 1;
                InitializeMapNodes();
                SetupFocusNavigation();
                StartCoroutine(AnimateNodeUnlock(currentUnlockedLevel));
            }
        }

        private IEnumerator AnimateNodeUnlock(int newLevel)
        {
            ScrollToNode(newLevel);
            yield return new WaitForSeconds(0.4f);

            Transform targetNode = levelNodeButtons[newLevel - 1].transform;
            Vector3 originalScale = targetNode.localScale;

            // Punch scale animation
            float elapsed = 0f;
            while (elapsed < 0.3f)
            {
                elapsed += Time.deltaTime;
                float progress = elapsed / 0.3f;
                targetNode.localScale = Vector3.Lerp(originalScale, originalScale * 1.3f, Mathf.Sin(progress * Mathf.PI));
                yield return null;
            }
            targetNode.localScale = originalScale;
        }

        private void ScrollToNode(int levelIndex)
        {
            if (mapScrollRect == null || levelIndex < 1 || levelIndex > levelNodeButtons.Length) return;

            RectTransform target = levelNodeButtons[levelIndex - 1].GetComponent<RectTransform>();
            float normalizedY = Mathf.Clamp01(target.anchoredPosition.y / (mapContent.rect.height - mapScrollRect.viewport.rect.height));
            mapScrollRect.verticalNormalizedPosition = normalizedY;
        }
    }
}
