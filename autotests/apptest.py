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

# Misc tests that aren't large enough (yet) to justify their own file
class ItineraryTest(ItineraryTestCase):
    def test_addTrainTrip(self):
        self.driver.find_element(by=AppiumBy.NAME, value="Plan Trip").click()

        self.driver.find_element(by=AppiumBy.NAME, value="From:").click()
        self.driver.find_element('description', value="Select country").click()
        # TODO we cannot scroll in the popup yet to get to Switzerland!
        self.driver.find_element(by=AppiumBy.NAME, value="Liechtenstein").click()
        searchField = self.driver.find_element(by=AppiumBy.NAME, value="Search")
        self.assertTrue(searchField.is_displayed())
        searchField.clear()
        searchField.send_keys("Randa")
        self.driver.find_element(by=AppiumBy.NAME, value="Randa, Valais, Switzerland").click()

        self.assertFalse(self.driver.find_element(by=AppiumBy.NAME, value="Search Journey").is_enabled())

        self.driver.find_element(by=AppiumBy.NAME, value="To:").click()
        # exists in history
        self.assertTrue(self.driver.find_element(by=AppiumBy.NAME, value="Randa, Valais, Switzerland").is_displayed())

        # clear history
        self.openContextDrawer()
        self.assertTrue(self.driver.find_element(by=AppiumBy.NAME, value="Clear history").is_enabled())
        self.driver.find_element(by=AppiumBy.NAME, value="Clear history").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Cancel").click()
        self.assertTrue(self.driver.find_element(by=AppiumBy.NAME, value="Randa, Valais, Switzerland").is_displayed())
        self.openContextDrawer()
        self.driver.find_element(by=AppiumBy.NAME, value="Clear history").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Remove").click()
        # self.assertFalse(self.driver.find_element(by=AppiumBy.NAME, value="Randa, Valais, Switzerland").is_displayed())
        self.goBack()


    def test_import(self):
        self.triggerImportAction("Scan Barcode…")
        self.assertTrue(self.driver.find_element(by=AppiumBy.NAME, value="No camera available.").is_displayed())
        self.assertFalse(self.driver.find_element(by=AppiumBy.NAME, value="Light").is_enabled())
        self.goBack()

    def test_statistics(self):
        self.openGlobalDrawer()
        self.driver.find_element(by=AppiumBy.NAME, value="Statistics").click()
        # TODO year combo box inaccessible
        self.goBack()

    def test_about(self):
        self.openGlobalDrawer()
        self.driver.find_element(by=AppiumBy.NAME, value="About").click()
        self.goBack()

if __name__ == '__main__':
    unittest.main()
