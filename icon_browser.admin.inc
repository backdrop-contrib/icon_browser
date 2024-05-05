<?php
/**
 * @file
 * Contains icon_browser.admin.inc
 *
 */

/**
 * Display the page of icons with filters.
 *
 * The icons are loaded first from core, then from modules, and then from the
 * active theme. This way if an icon is overridden, we show the correct one.
 */
function icon_browser_page() {
  $output = array();
  // @todo caching.
  $icons = [];
  // Core icons.
  $core_icons = scandir(ICON_BROWSER_PATH);
  foreach ($core_icons as $icon) {
    if (substr($icon, -4, 4) == '.svg') {
      $icon_name = substr($icon, 0, -4);
      $icons[$icon_name] = array(
        'provider' => 'core',
        'name' => $icon_name,
        'fill' => substr($icon_name, -5, 5) === '-fill',
        'directory' => ICON_BROWSER_PATH,
      );
    }
  }
  // Module icons.
  $module_icons = icon_get_info();
  foreach ($module_icons as $key => $icon) {
    $icons[$icon['name']] = array(
      'provider' => 'module',
      'provider_key' => $key,
      'name' => $icon['name'],
      'fill' => substr($icon['name'], -5, 5) === '-fill',
    );
  }
  // @todo Theme icons.
  $output = array(
    '#theme' => 'icon_browser_page',
    '#icons' => $icons,
    '#attached' => array(
      'css' => array(
        backdrop_get_path('module', 'icon_browser') . '/css/icon_browser.css',
      ),
    ),
  );
  return $output;
}

/**
 * Form for filtering icons.
 *
 * @param array $form
 *   An associative array containing the structure of a portion of the form.
 * @param array $form_state
 *   A keyed array containing the current state of the form.
 *
 */
function icon_browser_filters_form($form, &$form_state) {
  $form = array(
    'provider' => array(
      '#type' => 'select',
      '#title' => t('Provider'),
      '#options' => array(
        'any' => t('Any'),
        'core' => t('Core'),
        'module' => t('Module'),
      ),
      '#default_value' => icon_browser_get_filter_value('provider'),
    ),
    'style' => array(
      '#type' => 'select',
      '#title' => t('Style'),
      '#options' => array(
        'any' => t('Any'),
        'fill' => t('Fill'),
        'outline' => t('Outline or other'),
      ),
      '#default_value' => icon_browser_get_filter_value('style'),
    ),
    'keyword' => array(
      '#type' => 'textfield',
      '#title' => t('Keyword'),
      '#default_value' => icon_browser_get_filter_value('keyword'),
      '#size' => 10,
    ),
    'size' => array(
      '#type' => 'select',
      '#title' => t('Preview Size'),
      '#options' => array(
        'small' => t('Small'),
        'medium' => t('Medium'),
        'large' => t('Large'),
        'xlarge' => t('Extra Large'),
      ),
      '#default_value' => icon_browser_get_filter_value('size'),
    ),
    'actions' => array(
      '#type' => 'actions',
      'submit' => array(
        '#type' => 'submit',
        '#value' => t('Filter'),
      ),
      'reset' => array(
        '#type' => 'submit',
        '#value' => t('Reset'),
      ),
      'save' => array(
        '#type' => 'submit',
        '#value' => t('Save defaults'),
      ),
    ),
  );
  return $form;
}

/**
 * Handles the Icon Browser filter form submit.
 */
function icon_browser_filters_form_submit($form, &$form_state) {
  if ($form_state['triggering_element']['#parents'][0] == 'reset') {
    unset($_SESSION['icon_browser']);
  }
  else {
    if ($form_state['triggering_element']['#parents'][0] == 'save') {
      $config = config('icon_browser.settings');
      $config->set('provider', $form_state['values']['provider']);
      $config->set('style', $form_state['values']['style']);
      $config->set('keyword', $form_state['values']['keyword']);
      $config->set('size', $form_state['values']['size']);
      $config->save();
      backdrop_set_message(t('Defaults saved.'));
    }
    $_SESSION['icon_browser']['provider'] = $form_state['values']['provider'];
    $_SESSION['icon_browser']['style'] = $form_state['values']['style'];
    $_SESSION['icon_browser']['keyword'] = $form_state['values']['keyword'];
    $_SESSION['icon_browser']['size'] = $form_state['values']['size'];
  }
}

  /**
 * Theme hook for the icon browser page.
 */
function theme_icon_browser_page($variables) {
  $icon_name = check_plain(arg(5));
  $output = '';
  $title = (!empty($icon_name)) ? t('Icon: @icon', ['@icon' => $icon_name]) : t('Icon Browser');
  backdrop_set_title($title);
  $output .= '<div class="icon-browser">';
  // Details (if needed).
  if ($icon_name) {
    $output .= '<div class="icon-detail messages">';
    if (config_get('system.core', 'messages_dismissible')) {
      // Add the 'Dismiss' library and place a 'Dismiss' link on icon details.
      backdrop_add_library('system', 'backdrop.dismiss');
      $output .= '<a href="#" class="button button--dismiss dismiss" title="' . t('Dismiss') . '"><span class="icon-close"></span></a>' . "\n";
    }
    $output .= '<div class="icon-image">'. icon($icon_name) . '</div>';
    $output .= '<div class="icon-instructions">';
    $output .= '<div class="icon-name"><strong>Icon name:</strong> <pre>' . $icon_name . '</pre></div>';
    $output .= '<div class="icon-php"><strong>PHP:</strong> <pre>icon(\'' . $icon_name . '\');</pre></div>';
    $output .= '<div class="icon-css"><strong>CSS:</strong> <pre>background-image: var(--icon-' . $icon_name . ');</pre></div>';
    $output .= '<div class="icon-js"><strong>JavaScript:</strong> <pre>Backdrop.icons[\'' . $icon_name . '\']</pre></div>';
    $output .= '</div></div>';
  }
  // Filters.
  $output .= '<div class="icon-browser-filters">';
  $filters = backdrop_get_form('icon_browser_filters_form');
  $output .= render($filters);
  $output .= '</div>';
  // Icons.
  $provider = icon_browser_get_filter_value('provider');
  $style = icon_browser_get_filter_value('style');
  $keyword = icon_browser_get_filter_value('keyword');
  $size = icon_browser_get_filter_value('size');
  $output .= "<div class='icon-browser-list' data-icon-provider='$provider' data-icon-style='$style' data-icon-size='$size'>";
  foreach ($variables['icons'] as $icon) {
    if ($provider != 'any' && $icon['provider'] != $provider) {
      continue;
    }
    if ($style != 'any' && (($icon['fill'] && $style != 'fill') || (!$icon['fill'] && $style != 'outline'))) {
      continue;
    }
    if ($keyword && strpos($icon['name'], $keyword) === FALSE) {
      continue;
    }
    $output .= '<div class="icon-wrapper">';
    $output .= '<a href="/admin/config/media/icons/browse/' . $icon['provider_key'] . '">';
    $output .= '<div class="icon">' . icon($icon['provider_key']) . '</div>';
    $output .= '<div class="icon-name">' . $icon['name'] . '</div></a></div>';
  }
  $output .= '</div>';
  $output .= '</div>';
  return $output;
}

/**
 * Helper function to get current filter values for the current session.
 *
 * @param string $filter_name
 *
 * @return string
 */
function icon_browser_get_filter_value($filter_name) {
  return (!empty($_SESSION['icon_browser'][$filter_name])) ? $_SESSION['icon_browser'][$filter_name] : config_get('icon_browser.settings', $filter_name);
}
