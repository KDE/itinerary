#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-2.0-or-later
# SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>

from itinerarytestcase import ItineraryTestCase

import unittest
import time
from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy
import selenium.common.exceptions
from selenium.webdriver.common.action_chains import ActionChains
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.support.ui import WebDriverWait

import os

class TripGroupTest(ItineraryTestCase):
    def setUp(self):
        self.driver.find_element(by=AppiumBy.NAME, value="Itinerary").click()

    def testEmptyGroup(self):
        self.createAndEnterTrip("My New Trip")

        self.driver.find_element(by=AppiumBy.NAME, value="Add event").click()
        # TODO how can we check the A11y check state?
        # self.assertTrue(self.driver.find_element(by=AppiumBy.NAME, value="Add to Trip").is_checked())
        self.assertFalse(self.driver.find_element(by=AppiumBy.NAME, value="Save").is_enabled())

        self.driver.find_element(by=AppiumBy.NAME, value="Name").send_keys("Akademy 2024")
        saveBtn = self.driver.find_element(by=AppiumBy.NAME, value="Save")
        self.assertTrue(saveBtn.is_enabled())
        saveBtn.click()

        # HACK model doesn't update correctly
        # self.goBack()
        # self.driver.find_element(by=AppiumBy.NAME, value="My New Trip").click()

        self.deleteTrip("Akademy 2024")

    def testImportedGroup(self):
        self.driver.find_element(by=AppiumBy.NAME, value="MyData").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Import").click()

        self.openFile("../tests/randa2017.json")
        self.driver.find_element(by=AppiumBy.NAME, value="Import selection").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Randa (September 2017)").click()

        self.driver.find_element(by=AppiumBy.NAME, value="Delete trip").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Delete").click()

if __name__ == '__main__':
    unittest.main()
