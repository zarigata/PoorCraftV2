package com.poorcraft.common.mod;

import com.google.gson.JsonObject;
import com.google.gson.JsonParser;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.nio.charset.StandardCharsets;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Collections;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Objects;
import java.util.Set;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

public class ModMetadata {
    private static final Pattern ID_PATTERN = Pattern.compile("[a-z0-9_\\-]+");
    private static final Pattern VERSION_PATTERN = Pattern.compile("[0-9]+(\\.[0-9]+){0,2}(-[A-Za-z0-9]+)?");

    private final String id;
    private final String name;
    private final String version;
    private final String description;
    private final List<String> authors;
    private final List<ModDependency> dependencies;
    private final String minGameVersion;
    private final Set<String> permissions;
    private final String scriptFile;
    private final String mainClass;
    private transient Path sourcePath;

    public ModMetadata(JsonObject json) {
        Objects.requireNonNull(json, "json");
        this.id = getString(json, "id", null);
        this.name = getString(json, "name", id);
        this.version = getString(json, "version", "1.0.0");
        this.description = getString(json, "description", "");
        this.authors = getStringList(json, "authors");
        this.dependencies = getDependencies(json);
        this.minGameVersion = getString(json, "minGameVersion", "0.0.0");
        this.permissions = getPermissionSet(json);
        this.scriptFile = getString(json, "scriptFile", null);
        this.mainClass = getString(json, "mainClass", null);
    }

    public static ModMetadata fromJson(InputStream stream) throws IOException {
        try (Reader reader = new InputStreamReader(stream, StandardCharsets.UTF_8)) {
            JsonObject json = JsonParser.parseReader(reader).getAsJsonObject();
            return new ModMetadata(json);
        }
    }

    public void validate() {
        if (id == null || !ID_PATTERN.matcher(id).matches()) {
            throw new IllegalArgumentException("Invalid mod id: " + id);
        }
        if (version == null || !VERSION_PATTERN.matcher(version).matches()) {
            throw new IllegalArgumentException("Invalid version for mod '" + id + "': " + version);
        }
        if (minGameVersion != null && !VERSION_PATTERN.matcher(minGameVersion).matches()) {
            throw new IllegalArgumentException("Invalid minimum game version for mod '" + id + "': " + minGameVersion);
        }
        Set<String> duplicateCheck = new LinkedHashSet<>();
        for (ModDependency dependency : dependencies) {
            if (!duplicateCheck.add(dependency.getModId())) {
                throw new IllegalArgumentException("Duplicate dependency '" + dependency.getModId() + "' for mod '" + id + "'");
            }
        }
    }

    public String getId() {
        return id;
    }

    public String getName() {
        return name;
    }

    public String getVersion() {
        return version;
    }

    public String getDescription() {
        return description;
    }

    public List<String> getAuthors() {
        return authors;
    }

    public List<ModDependency> getDependencies() {
        return dependencies;
    }

    public String getMinGameVersion() {
        return minGameVersion;
    }

    public Set<String> getPermissions() {
        return permissions;
    }

    public String getScriptFile() {
        return scriptFile;
    }

    public String getMainClass() {
        return mainClass;
    }

    public Path getSourcePath() {
        return sourcePath;
    }

    public void setSourcePath(Path sourcePath) {
        this.sourcePath = sourcePath;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) {
            return true;
        }
        if (!(o instanceof ModMetadata)) {
            return false;
        }
        ModMetadata that = (ModMetadata) o;
        return id.equals(that.id);
    }

    @Override
    public int hashCode() {
        return id.hashCode();
    }

    private static String getString(JsonObject json, String member, String defaultValue) {
        if (json.has(member) && !json.get(member).isJsonNull()) {
            return json.get(member).getAsString();
        }
        return defaultValue;
    }

    private static List<String> getStringList(JsonObject json, String member) {
        if (!json.has(member) || json.get(member).isJsonNull()) {
            return Collections.emptyList();
        }
        List<String> result = new ArrayList<>();
        json.get(member).getAsJsonArray().forEach(element -> result.add(element.getAsString()));
        return Collections.unmodifiableList(result);
    }

    private static Set<String> getPermissionSet(JsonObject json) {
        if (!json.has("permissions") || json.get("permissions").isJsonNull()) {
            return Collections.emptySet();
        }
        return json.get("permissions").getAsJsonArray().stream()
                .map(element -> element.getAsString().toLowerCase())
                .collect(Collectors.toCollection(LinkedHashSet::new));
    }

    private static List<ModDependency> getDependencies(JsonObject json) {
        if (!json.has("dependencies") || json.get("dependencies").isJsonNull()) {
            return Collections.emptyList();
        }
        List<ModDependency> result = new ArrayList<>();
        json.get("dependencies").getAsJsonArray().forEach(element -> {
            JsonObject dep = element.getAsJsonObject();
            result.add(new ModDependency(
                    dep.get("modId").getAsString(),
                    getString(dep, "versionRange", ""),
                    dep.has("optional") && dep.get("optional").getAsBoolean()
            ));
        });
        return Collections.unmodifiableList(result);
    }

    public static class ModDependency {
        private final String modId;
        private final String versionRange;
        private final boolean optional;

        public ModDependency(String modId, String versionRange, boolean optional) {
            this.modId = Objects.requireNonNull(modId, "modId");
            this.versionRange = versionRange == null ? "" : versionRange;
            this.optional = optional;
        }

        public String getModId() {
            return modId;
        }

        public String getVersionRange() {
            return versionRange;
        }

        public boolean isOptional() {
            return optional;
        }
    }
}
