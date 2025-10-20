package com.poorcraft.common.util;

import org.slf4j.LoggerFactory;

/**
 * Centralized logging utility that wraps SLF4J.
 * Provides a consistent logging interface for the entire project.
 */
public class Logger {
    
    /**
     * Gets a logger for the specified class.
     * 
     * @param clazz The class to get a logger for
     * @return An SLF4J logger instance
     */
    public static org.slf4j.Logger getLogger(Class<?> clazz) {
        return LoggerFactory.getLogger(clazz);
    }
    
    /**
     * Gets a logger with the specified name.
     * 
     * @param name The logger name
     * @return An SLF4J logger instance
     */
    public static org.slf4j.Logger getLogger(String name) {
        return LoggerFactory.getLogger(name);
    }
    
    /**
     * Checks if debug logging is enabled for the specified class.
     * Use this before expensive string operations.
     * 
     * @param clazz The class to check
     * @return true if debug is enabled
     */
    public static boolean isDebugEnabled(Class<?> clazz) {
        return getLogger(clazz).isDebugEnabled();
    }
    
    /**
     * Checks if trace logging is enabled for the specified class.
     * Use this before expensive string operations.
     * 
     * @param clazz The class to check
     * @return true if trace is enabled
     */
    public static boolean isTraceEnabled(Class<?> clazz) {
        return getLogger(clazz).isTraceEnabled();
    }
}
