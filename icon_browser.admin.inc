<?php
/**
 * @file
 * Contains icon_browser.admin.inc
 *
 */

define ('ICON_BROWSER_PER_PAGE', 110);

/**
 * Display the page of icons with filters.
 *
 * The icons are loaded first from core, then from modules, and then from the
 * active theme. This way if an icon is overridden, we show the correct one.
 */
function icon_browser_page() {
  $output = array();
  $icons = icon_browser_icon_details();
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
        'theme' => t('Theme'),
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
      $config->set('size', $form_state['values']['size']);
      $config->save();
      backdrop_set_message(t('Defaults saved.'));
    }
    $_SESSION['icon_browser']['provider'] = $form_state['values']['provider'];
    $_SESSION['icon_browser']['style'] = $form_state['values']['style'];
    $_SESSION['icon_browser']['keyword'] = $form_state['values']['keyword'];
    $_SESSION['icon_browser']['size'] = $form_state['values']['size'];
  }
  // Redirect back to the main form to remove any detail view.
  $form_state['redirect'] = 'admin/config/media/icons/browse';
}

  /**
 * Theme hook for the icon browser page.
 */
function theme_icon_browser_page($variables) {
  backdrop_add_icons(array('seal-warning-fill'));
  $icon_key = check_plain(arg(5));
  $provider = icon_browser_get_filter_value('provider');
  $style = icon_browser_get_filter_value('style');
  $keyword = icon_browser_get_filter_value('keyword');
  $size = icon_browser_get_filter_value('size');
  $results = array();
  foreach ($variables['icons'] as $icon) {
    if ($provider != 'any' && $icon['provider'] != $provider) {
      continue;
    }
    if ($style != 'any' && (($icon['fill'] && $style != 'fill') || (!$icon['fill'] && $style != 'outline'))) {
      continue;
    }
    $searchtext = implode(' ', $icon);
    if ($keyword && strpos($searchtext, $keyword) === FALSE) {
      continue;
    }
    $results[] = $icon;
  }
  $current_page = pager_default_initialize(count($results), ICON_BROWSER_PER_PAGE);
  $chunks = array_chunk($results, ICON_BROWSER_PER_PAGE, TRUE);

  $output = '';
  $title = (!empty($icon_key)) ? t('Icon: @icon', ['@icon' => $icon_key]) : t('Icon Browser');
  backdrop_set_title($title);
  $output .= '<div class="icon-browser">';
  // Details (if needed).
  if ($icon_key && $results) {
    $icon_details = icon_browser_icon_details($icon_key);
    if (!empty($icon_details)) {
      $output .= '<div class="icon-browser-detail messages" data-icon-name="' . $icon_key . '" data-icon-active="false">';
      if (config_get('system.core', 'messages_dismissible')) {
        // Add the 'Dismiss' library and place a 'Dismiss' link on icon details.
        backdrop_add_library('system', 'backdrop.dismiss');
        $output .= '<a href="#" class="button button--dismiss dismiss" title="' . t('Dismiss') . '"><span class="icon-close"></span></a>' . "\n";
      }
      if (empty($icon_details['error']) || !$icon_details['error']) {
        $output .= '<div class="icon-image">'. icon($icon_key) . '</div>';
      }
      else {
        $output .= '<div class="icon-image error">'. icon('seal-warning-fill') . '</div>';
      }
      $output .= '<div class="icon-instructions">';
      $output .= '<div class="icon-name"><h2>' . $icon_key . '</h2>';
      $output .= '<p><strong>Provided by:</strong> ' . $icon_details['provider_detail'] . '</p>';
      $output .= '<p class="icon-status"><strong>CSS/JS Status:</strong> <span>' . t('Inactive') . '</span>';
      $output .= '</div>';
      $output .= '<div class="icon-php"><label class="icon-browser-ready">PHP:</label> <pre>icon(\'' . $icon_key . '\');</pre></div>';
      $output .= '<div class="icon-css"><label>CSS:</label> <pre>background-image: var(--icon-' . $icon_key . ');</pre></div>';
      $output .= '<div class="icon-js"><label>JavaScript:</label> <pre>Backdrop.icons[\'' . $icon_key . '\']</pre></div>';
      $output .= '</div></div>';
    }
  }
  // Filters.
  $output .= '<div class="icon-browser-filters">';
  $filters = backdrop_get_form('icon_browser_filters_form');
  $output .= render($filters);
  $output .= '</div>';
  // Icons.
  if ($results) {
    $output .= "<div class='icon-browser-list' data-icon-provider='$provider' data-icon-style='$style' data-icon-size='$size'>";

    foreach ($chunks[$current_page] as $icon) {
      $error_class = (!empty($icon['error'])) ? ' icon-error' : '';
      $output .= '<div class="icon-wrapper' . $error_class . '" data-icon-name="' . $icon['provider_key'] . '" data-icon-active="false">';
      $url = url('admin/config/media/icons/browse/' . $icon['provider_key'], array('query' => array('page' => $current_page)));
      $output .= '<a href="' . $url . '">';
      $output .= '<div class="icon">' . icon($icon['provider_key']) . '</div>';
      $output .= '<div class="icon-name">' . $icon['name'] . '</div></a></div>';
    }
    $output .= '</div>';
  }
  else {
    $output .= t('No icons match the filter criteria.');
  }
  $output .= '</div>';
  $output .= theme('pager', array('quantity', count($results)));
  backdrop_add_icons(array('check-circle-fill', 'warning-circle-fill'));
  backdrop_add_js(backdrop_get_path('module', 'icon_browser') . '/js/icon_browser.js');

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


/**
 * Helper function to load icon details.
 *
 * @param string $icon_key
 *   The icon key to load details for. If empty, load all icons.
 * @return array
 *   An array of icon details, keyed by icon key:
 *    - All icons, if no $icon_key was provided
 *    - The single result if an $icon_key was provided, or
 *    - An empty array if no icon was found for a provided $icon_key.
 */
function icon_browser_icon_details($icon_key = '') {
  // @todo caching.
  $icons = [];
  // Icons provided by Backdrop core.
  $core_icons = scandir(ICON_BROWSER_PATH);
  foreach ($core_icons as $icon) {
    if (substr($icon, -4, 4) == '.svg') {
      $icon_name = substr($icon, 0, -4);
      $icons[$icon_name] = array(
        'provider' => 'core',
        'provider_detail' => t('Backdrop core'),
        'provider_key' => $icon_name,
        'name' => $icon_name,
        'fill' => substr($icon_name, -5, 5) === '-fill',
        'directory' => ICON_BROWSER_PATH,
      );
    }
  }
  // Icons provided by modules.
  $module_icons = icon_get_info();
  foreach ($module_icons as $key => $icon) {
    $icons[$key] = array(
      'provider' => 'module',
      'provider_detail' => t('@module (module)', ['@module' => $icon['module']]),
      'provider_key' => $key,
      'name' => $icon['name'],
      'fill' => substr($icon['name'], -5, 5) === '-fill',
    );
  }
  // Icons provided by themes.
  $themes = list_themes();
  $active_theme = $GLOBALS['theme_key'];
  foreach ($themes as $theme => $theme_data) {
    $error = ($theme != $active_theme);
    // Check if the theme provides a non-default icon path.
    $theme_icon_directory = theme_get_setting('icon_directory', $theme);

    // Otherwise just check in the theme "icons" directory.
    if (!$theme_icon_directory) {
      $theme_icon_directory = 'icons';
    }
    $theme_directory = backdrop_get_path('theme', $theme) . '/' . $theme_icon_directory;
    if (is_dir($theme_directory)) {
      $theme_icons = scandir($theme_directory);
      if (!empty($theme_icons)) {
        foreach ($theme_icons as $key => $icon) {
          if (substr($icon, -4, 4) == '.svg') {
            $icon_name = substr($icon, 0, -4);
            $icons[$icon_name] = array(
              'provider' => 'theme',
              'provider_detail' => t('@theme (theme*) <br><br><em>*Theme-provided icons may only be available when the theme is active.</em>', ['@theme' => $theme]),
              'provider_key' => $icon_name,
              'name' => $icon_name,
              'fill' => substr($icon_name, -5, 5) === '-fill',
              'directory' => $theme_directory,
              'error' => $error,
            );
          }
        }
      }
    }
  }
  if (empty($icon_key)) {
    $result = $icons;
  }
  elseif (isset($icons[$icon_key])) {
    $result = $icons[$icon_key];
  }
  else {
    $result = [];
  }
  return $result;
}
