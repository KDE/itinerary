#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-2.0-or-later
# SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>

from itinerarytestcase import ItineraryTestCase

import unittest
from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy
import selenium.common.exceptions
from selenium.webdriver.common.action_chains import ActionChains
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.support.ui import WebDriverWait

import os

class SettingsTest(ItineraryTestCase):
    def test_settings(self):
        self.openGlobalDrawer()
        self.driver.find_element(by=AppiumBy.NAME, value="Settings...").click()

        # TODO home country combo box not accessible

        self.driver.find_element(by=AppiumBy.NAME, value="Favorite Locations").click()
        # TODO favorite location combo box inaccessbile
        self.goBack()

        trafficData = self.driver.find_element(by=AppiumBy.NAME, value="Query Traffic Data")
        self.assertEqual(trafficData.get_attribute("checked"), "false")
        trafficData.send_keys(Keys.SPACE) # TODO there is no toggle()?
        self.assertEqual(trafficData.get_attribute("checked"), "true")

        self.driver.find_element(by=AppiumBy.NAME, value="Public Transport Information Sources...").click()
        self.goBack()
        self.goBack()

        self.openGlobalDrawer()
        self.driver.find_element(by=AppiumBy.NAME, value="Settings...").click()
        self.assertEqual(self.driver.find_element(by=AppiumBy.NAME, value="Query Traffic Data").get_attribute("checked"), "true")
        self.goBack()

if __name__ == '__main__':
    unittest.main()
