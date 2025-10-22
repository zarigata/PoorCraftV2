package com.poorcraft.common.mod;

import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Deque;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Set;

public final class DependencyResolver {
    private DependencyResolver() {
    }

    public static List<ModMetadata> resolve(List<ModMetadata> mods) throws DependencyException {
        Objects.requireNonNull(mods, "mods");
        Map<String, ModMetadata> byId = new LinkedHashMap<>();
        for (ModMetadata metadata : mods) {
            String id = metadata.getId();
            if (byId.putIfAbsent(id, metadata) != null) {
                throw new DependencyException("Duplicate mod id detected: " + id);
            }
        }

        Map<String, Set<String>> graph = new HashMap<>();
        Map<String, Integer> indegree = new HashMap<>();

        for (ModMetadata metadata : mods) {
            String id = metadata.getId();
            graph.computeIfAbsent(id, key -> new HashSet<>());
            indegree.putIfAbsent(id, 0);
            for (ModMetadata.ModDependency dependency : metadata.getDependencies()) {
                String depId = dependency.getModId();
                ModMetadata depMetadata = byId.get(depId);
                if (depMetadata == null) {
                    if (dependency.isOptional()) {
                        continue;
                    }
                    throw new DependencyException("Mod '" + id + "' is missing required dependency '" + depId + "'");
                }
                if (!validateVersionConstraint(depMetadata.getVersion(), dependency.getVersionRange())) {
                    throw new DependencyException("Mod '" + id + "' depends on '" + depId + "' with version constraint '"
                            + dependency.getVersionRange() + "' but version '" + depMetadata.getVersion() + "' is installed");
                }
                graph.computeIfAbsent(depId, key -> new HashSet<>()).add(id);
                indegree.put(id, indegree.getOrDefault(id, 0) + 1);
            }
        }

        Deque<String> queue = new ArrayDeque<>();
        for (Map.Entry<String, Integer> entry : indegree.entrySet()) {
            if (entry.getValue() == 0) {
                queue.add(entry.getKey());
            }
        }

        List<ModMetadata> sorted = new ArrayList<>();
        while (!queue.isEmpty()) {
            String id = queue.remove();
            ModMetadata metadata = byId.get(id);
            sorted.add(metadata);
            Set<String> dependencies = graph.getOrDefault(id, Collections.emptySet());
            for (String depId : dependencies) {
                int count = indegree.merge(depId, -1, Integer::sum);
                if (count == 0) {
                    queue.add(depId);
                }
            }
        }

        if (sorted.size() != mods.size()) {
            throw new DependencyException("Circular dependency detected among mods");
        }

        return sorted;
    }

    static boolean validateVersionConstraint(String version, String constraint) {
        if (constraint == null || constraint.isBlank()) {
            return true;
        }
        String[] parts = constraint.split(",");
        for (String part : parts) {
            String trimmed = part.trim();
            if (trimmed.isEmpty()) {
                continue;
            }
            if (trimmed.startsWith(">=")) {
                if (compareVersions(version, trimmed.substring(2).trim()) < 0) {
                    return false;
                }
            } else if (trimmed.startsWith("<=")) {
                if (compareVersions(version, trimmed.substring(2).trim()) > 0) {
                    return false;
                }
            } else if (trimmed.startsWith(">")) {
                if (compareVersions(version, trimmed.substring(1).trim()) <= 0) {
                    return false;
                }
            } else if (trimmed.startsWith("<")) {
                if (compareVersions(version, trimmed.substring(1).trim()) >= 0) {
                    return false;
                }
            } else if (trimmed.startsWith("==")) {
                if (compareVersions(version, trimmed.substring(2).trim()) != 0) {
                    return false;
                }
            } else {
                // treat bare version as equality requirement
                if (compareVersions(version, trimmed) != 0) {
                    return false;
                }
            }
        }
        return true;
    }

    private static int compareVersions(String left, String right) {
        int[] a = parseVersion(left);
        int[] b = parseVersion(right);
        int length = Math.max(a.length, b.length);
        for (int i = 0; i < length; i++) {
            int ai = i < a.length ? a[i] : 0;
            int bi = i < b.length ? b[i] : 0;
            if (ai != bi) {
                return Integer.compare(ai, bi);
            }
        }
        return 0;
    }

    private static int[] parseVersion(String version) {
        if (version == null || version.isBlank()) {
            return new int[]{0};
        }
        String numeric = version.split("-")[0];
        String[] parts = numeric.split("\\.");
        int[] result = new int[parts.length];
        for (int i = 0; i < parts.length; i++) {
            try {
                result[i] = Integer.parseInt(parts[i]);
            } catch (NumberFormatException e) {
                result[i] = 0;
            }
        }
        return result;
    }

    public static class DependencyException extends Exception {
        public DependencyException(String message) {
            super(message);
        }
    }
}
